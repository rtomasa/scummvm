/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef __LIBRETRO__

// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
// Also with clock() in sys/time.h in some Mac OS X SDKs.
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_exit // Needed for IRIX's unistd.h

#include "backends/platform/libretro/include/libretro-fs-factory.h"
#include "backends/platform/libretro/include/libretro-fs.h"

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/ptr.h"
#include "common/stream.h"

class LibRetroGameArchive {
	struct CachedFile {
		Common::Path path;
		Common::SharedPtr<Common::SeekableReadStream> stream;
	};
	Common::Array<CachedFile> _cache;
	CachedFile _largeFile;
	uint64 _cachedBytes = 0;

public:
	Common::SharedPtr<Common::Archive> archive;

	explicit LibRetroGameArchive(Common::Archive *zip) : archive(zip) {}

	Common::SeekableReadStream *open(const Common::Path &path) {
		Common::SeekableReadStream *stream = archive->createReadStreamForMember(path);
		if (!stream)
			return nullptr;

		// Keep a stream alive so the ZIP reader's weak cache retains its data.
		const uint64 budget = 128 * 1024 * 1024;
		if ((uint64)stream->size() > budget) {
			if (!_largeFile.stream || !Common::Path::IgnoreCase_EqualTo()(_largeFile.path, path)) {
				_largeFile.stream.reset();
				_largeFile.path = path;
				_largeFile.stream.reset(archive->createReadStreamForMember(path));
			}
		} else {
			for (uint i = 0; i < _cache.size(); ++i) {
				if (Common::Path::IgnoreCase_EqualTo()(_cache[i].path, path)) {
					CachedFile entry = _cache[i];
					_cache.remove_at(i);
					_cache.push_back(entry);
					return stream;
				}
			}
			while (!_cache.empty() && _cachedBytes + stream->size() > budget) {
				_cachedBytes -= _cache[0].stream->size();
				_cache.remove_at(0);
			}
			CachedFile entry;
			entry.path = path;
			entry.stream.reset(archive->createReadStreamForMember(path));
			if (entry.stream) {
				_cachedBytes += entry.stream->size();
				_cache.push_back(entry);
			}
		}
		return stream;
	}
};

static Common::SharedPtr<LibRetroGameArchive> s_gameArchive;
static Common::String s_gameArchivePath;

class LibRetroArchiveNode final : public AbstractFSNode {
	Common::SharedPtr<LibRetroGameArchive> _archive;
	Common::String _root;
	Common::Path _member;

public:
	LibRetroArchiveNode(Common::SharedPtr<LibRetroGameArchive> archive, const Common::String &root, const Common::Path &member)
		: _archive(archive), _root(root), _member(member) {}

	bool exists() const override { return isDirectory() || _archive->archive->hasFile(_member); }
	bool isDirectory() const override {
		Common::Array<Common::String> children;
		return _member.empty() || _archive->archive->getChildren(_member, children, Common::Archive::kListAll);
	}
	bool isReadable() const override { return exists(); }
	bool isWritable() const override { return false; }
	Common::String getName() const override { return _member.empty() ? Common::Path(_root).baseName() : _member.baseName(); }
	Common::U32String getDisplayName() const override { return getName(); }
	Common::String getPath() const override { return _member.empty() ? _root : _root + "/" + _member.toString('/'); }
	AbstractFSNode *getChild(const Common::String &name) const override {
		if (name.empty() || name == "." || name == ".." || name.contains('/') || name.contains('\\'))
			return nullptr;
		return new LibRetroArchiveNode(_archive, _root, _member.appendComponent(name));
	}
	AbstractFSNode *getParent() const override {
		if (_member.empty())
			return nullptr;
		return new LibRetroArchiveNode(_archive, _root, _member.getParent());
	}
	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override {
		Common::Array<Common::String> children;
		if (!_archive->archive->getChildren(_member, children, Common::Archive::kListAll, hidden))
			return false;
		for (const auto &name : children) {
			AbstractFSNode *child = getChild(name);
			if (!child)
				continue;
			if (mode == Common::FSNode::kListAll ||
			    (child->isDirectory() ? mode == Common::FSNode::kListDirectoriesOnly : mode == Common::FSNode::kListFilesOnly))
				list.push_back(child);
			else
				delete child;
		}
		return true;
	}
	Common::SeekableReadStream *createReadStream() override {
		return isDirectory() ? nullptr : _archive->open(_member);
	}
	Common::SeekableReadStream *createReadStreamForAltStream(Common::AltStreamType type) override {
		return _archive->archive->createReadStreamForMemberAltStream(_member, type);
	}
	Common::SeekableWriteStream *createWriteStream(bool atomic) override { return nullptr; }
	bool createDirectory() override { return false; }
};

void libretroUnmountArchive() {
	s_gameArchive.reset();
	s_gameArchivePath.clear();
}

bool libretroMountArchive(const Common::String &path) {
	libretroUnmountArchive();
	LibRetroFilesystemNode file(path);
	Common::Archive *zip = Common::makeZipArchive(file.createReadStream());
	if (!zip)
		return false;
	s_gameArchive.reset(new LibRetroGameArchive(zip));

	// Reject paths that could escape the archive's virtual root.
	Common::ArchiveMemberList members;
	s_gameArchive->archive->listMembers(members);
	for (const auto &member : members) {
		Common::String name = member->getPathInArchive().toString('/');
		if (name.hasPrefix("/") || name.contains('\\') || name.contains(':') ||
		    name == ".." || name.hasPrefix("../") || name.contains("/../") || name.hasSuffix("/..")) {
			libretroUnmountArchive();
			return false;
		}
	}
	s_gameArchivePath = file.getPath();
	return true;
}

AbstractFSNode *LibRetroFilesystemFactory::makeRootFileNode() const {
	if (LibRetroFilesystemNode::useAuthorizedRoot())
		return new LibRetroFilesystemNode(LibRetroFilesystemNode::getAuthorizedRootPath());

	return new LibRetroFilesystemNode("/");
}

AbstractFSNode *LibRetroFilesystemFactory::makeCurrentDirectoryFileNode() const {
#ifdef PLAYSTATION3
	return new LibRetroFilesystemNode("/");
#else
	return new LibRetroFilesystemNode(LibRetroFilesystemNode::getDefaultDir());
#endif
}

AbstractFSNode *LibRetroFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	if (s_gameArchive && (path == s_gameArchivePath || path.hasPrefix(s_gameArchivePath + "/"))) {
		Common::String member = path == s_gameArchivePath ? Common::String() : path.substr(s_gameArchivePath.size() + 1);
		return new LibRetroArchiveNode(s_gameArchive, s_gameArchivePath, Common::Path(member));
	}
	return new LibRetroFilesystemNode(path);
}
#endif
