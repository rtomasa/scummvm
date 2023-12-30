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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/puzzledata.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/nancy.h"

namespace Nancy {

SliderPuzzleData::SliderPuzzleData() : playerHasTriedPuzzle(false) {}

void SliderPuzzleData::synchronize(Common::Serializer &ser) {
	ser.syncAsByte(playerHasTriedPuzzle);

	byte x = 0, y = 0;

	if (ser.isSaving()) {
		y = playerTileOrder.size();
		if (y) {
			x = playerTileOrder.back().size();
		} else {
			x = 0;
		}
	}

	ser.syncAsByte(x);
	ser.syncAsByte(y);

	playerTileOrder.resize(y);

	for (int i = 0; i < y; ++i) {
		playerTileOrder[i].resize(x);
		ser.syncArray(playerTileOrder[i].data(), x, Common::Serializer::Sint16LE);
	}
}

RippedLetterPuzzleData::RippedLetterPuzzleData() :
	order(24, 0),
	rotations(24, 0),
	playerHasTriedPuzzle(false) {}

void RippedLetterPuzzleData::synchronize(Common::Serializer &ser) {
	if (ser.isLoading()) {
		order.resize(24);
		rotations.resize(24);
	}

	ser.syncArray(order.data(), 24, Common::Serializer::Byte);
	ser.syncArray(rotations.data(), 24, Common::Serializer::Byte);
}

TowerPuzzleData::TowerPuzzleData() {
	order.resize(3, Common::Array<int8>(6, -1));
	playerHasTriedPuzzle = false;
}

void TowerPuzzleData::synchronize(Common::Serializer &ser) {
	ser.syncAsByte(playerHasTriedPuzzle);

	if (ser.isLoading()) {
		order.resize(3, Common::Array<int8>(6, -1));
	}

	for (uint i = 0; i < 3; ++i) {
		ser.syncArray(order[i].data(), 6, Common::Serializer::Byte);
	}
}

RiddlePuzzleData::RiddlePuzzleData() :
	incorrectRiddleID(-1) {}

void RiddlePuzzleData::synchronize(Common::Serializer &ser) {
	byte numRiddles = solvedRiddleIDs.size();
	ser.syncAsByte(numRiddles);

	if (ser.isLoading()) {
		solvedRiddleIDs.resize(numRiddles);
	}

	ser.syncArray(solvedRiddleIDs.data(), numRiddles, Common::Serializer::Byte);
}

SoundEqualizerPuzzleData::SoundEqualizerPuzzleData() {
	sliderValues.resize(6, 255);
}

void SoundEqualizerPuzzleData::synchronize(Common::Serializer &ser) {
	ser.syncArray(sliderValues.data(), 6, Common::Serializer::Byte);
}

void JournalData::synchronize(Common::Serializer &ser) {
	uint16 numEntries = journalEntries.size();
	ser.syncAsUint16LE(numEntries);

	if (ser.isLoading()) {
		for (uint i = 0; i < numEntries; ++i) {
			uint16 id = 0;
			ser.syncAsUint16LE(id);
			uint16 numStrings = 0;
			ser.syncAsUint16LE(numStrings);
			auto &entry = journalEntries[id];
			for (uint j = 0; j < numStrings; ++j) {
				Common::String l;
				ser.syncString(l);
				entry.push_back(l);
			}
		}
	} else {
		for (auto &a : journalEntries) {
			uint16 id = a._key;
			ser.syncAsUint16LE(id);
			uint16 numStrings = a._value.size();
			ser.syncAsUint16LE(numStrings);
			for (uint i = 0; i < numStrings; ++i) {
				ser.syncString(a._value[i]);
			}
		}
	}
}

TableData::TableData() {
	auto *tabl = GetEngineData(TABL);
	assert(tabl);

	currentIDs = tabl->startIDs;
}

void TableData::synchronize(Common::Serializer &ser) {
	byte num = currentIDs.size();
	ser.syncAsByte(num);

	if (ser.isLoading()) {
		currentIDs.resize(num);
	}

	ser.syncArray(currentIDs.data(), num, Common::Serializer::Uint16LE);
}

PuzzleData *makePuzzleData(const uint32 tag) {
	switch(tag) {
	case SliderPuzzleData::getTag():
		return new SliderPuzzleData();
	case RippedLetterPuzzleData::getTag():
		return new RippedLetterPuzzleData();
	case TowerPuzzleData::getTag():
		return new TowerPuzzleData();
	case RiddlePuzzleData::getTag():
		return new RiddlePuzzleData();
	case SoundEqualizerPuzzleData::getTag():
		return new SoundEqualizerPuzzleData();
	case JournalData::getTag():
		return new JournalData();
	case TableData::getTag():
		return new TableData();
	default:
		return nullptr;
	}
}

} // End of namespace Nancy
