/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/core.cpp $
 * $Id: core.cpp 229 2011-02-12 06:50:14Z dreammaster $
 *
 */

#include "common/system.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "engines/engine.h"
#include "gui/saveload.h"
#include "tsage/tsage.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/events.h"
#include "tsage/scenes.h"
#include "tsage/staticres.h"
#include "tsage/globals.h"

namespace tSage {

// The engine uses ScumMVM screen buffering, so all logic is hardcoded to use pane buffer 0
#define CURRENT_PANENUM 0

/*--------------------------------------------------------------------------*/

InvObject::InvObject(int sceneNumber, int rlbNum, int cursorNum, CursorType cursorId, const Common::String description):
		_sceneNumber(sceneNumber), _rlbNum(rlbNum), _cursorNum(cursorNum), _cursorId(cursorId),
		_description(description) {
	_displayResNum = 3;
	_iconResNum = 5;

	// Decode the image for the inventory item to get it's display bounds
	uint size;
	byte *imgData = _vm->_dataManager->getSubResource(_displayResNum, _rlbNum, _cursorNum, &size);
	GfxSurface s = surfaceFromRes(imgData);
	_bounds = s.getBounds();

	DEALLOCATE(imgData);
}

void InvObject::setCursor() {
	_globals->_events._currentCursor = _cursorId;

	if (_iconResNum != -1) {
		GfxSurface s = surfaceFromRes(_iconResNum, _rlbNum, _cursorNum);
		
		Graphics::Surface src = s.lockSurface();
		_globals->_events.setCursor(src, s._transColour, s._centroid, _cursorId);
	}
}

/*--------------------------------------------------------------------------*/

InvObjectList::InvObjectList():
		_stunner(2280, 1, 2, OBJECT_STUNNER, "This is your stunner."),
		_scanner(1, 1, 3, OBJECT_SCANNER, "A combination scanner comm unit."),
		_stasisBox(5200, 1, 4, OBJECT_STASIS_BOX, "A stasis box."),
		_infoDisk(40, 1, 1, OBJECT_INFODISK, "The infodisk you took from the assassin."),
		_stasisNegator(0, 2, 2, OBJECT_STASIS_NEGATOR, "The stasis field negator."),
		_keyDevice(4250, 1, 6, OBJECT_KEY_DEVICE, "A magnetic key device."),
		_medkit(2280, 1, 7, OBJECT_MEDKIT,  "Your medkit."),
		_ladder(4100, 1, 8, OBJECT_LADDER, "The chief's ladder."),
		_rope(4150, 1, 9, OBJECT_ROPE, "The chief's rope."),
		_key(7700, 1, 11, OBJECT_KEY, "A key."),
		_translator(7700, 1, 13, OBJECT_TRANSLATOR,  "The dolphin translator box."),
		_ale(2150, 1, 10, OBJECT_ALE, "A bottle of ale."),
		_paper(7700, 1, 12, OBJECT_PAPER, "A slip of paper with the numbers 2,4, and 3 written on it."),
		_waldos(0, 1, 14, OBJECT_WALDOS, "A pair of waldos from the ruined probe."),
		_stasisBox2(8100, 1, 4, OBJECT_STASIS_BOX2, "A stasis box."),
		_ring(8100, 2, 5, OBJECT_RING, "This is a signet ring sent to you by Louis Wu."),
		_cloak(9850, 2, 6, OBJECT_CLOAK, "A fine silk cloak."),
		_tunic(9450, 2, 7, OBJECT_TUNIC, "The patriarch's soiled tunic."),
		_candle(9500, 2, 8, OBJECT_CANDLE, "A tallow candle."),
		_straw(9400, 2, 9, OBJECT_STRAW, "Clean, dry straw."),
		_scimitar(9850, 1, 18, OBJECT_SCIMITAR, "A scimitar from the Patriarch's closet."),
		_sword(9850, 1, 17, OBJECT_SWORD, "A short sword from the Patriarch's closet."),
		_helmet(9500, 2, 4, OBJECT_HELMET, "Some type of helmet."),
		_items(4300, 2, 10, OBJECT_ITEMS, "Two interesting items from the Tnuctipun vessel."),
		_concentrator(4300, 2, 11, OBJECT_CONCENTRATOR, "The Tnuctipun anti-matter concentrator contained in a stasis field."),
		_nullifier(5200, 2, 12, OBJECT_NULLIFIER, "A purported neural wave nullifier."),
		_peg(4045, 2, 16, OBJECT_PEG, "A peg with a symbol."),
		_vial(5100, 2, 17, OBJECT_VIAL, "A vial of the bat creatures anti-pheromone drug."),
		_jacket(9850, 3, 1, OBJECT_JACKET, "A natty padded jacket."),
		_tunic2(9850, 3, 2, OBJECT_TUNIC2, "A very hairy tunic."),
		_bone(5300, 3, 5, OBJECT_BONE, "A very sharp bone."),
		_jar(7700, 3, 4, OBJECT_JAR, "An jar filled with a green substance."),
		_emptyJar(7700, 3, 3, OBJECT_EMPTY_JAR, "An empty jar.") {

	// Add the items to the list
	_itemList.push_back(&_stunner);
	_itemList.push_back(&_scanner);
	_itemList.push_back(&_stasisBox);
	_itemList.push_back(&_infoDisk);
	_itemList.push_back(&_stasisNegator);
	_itemList.push_back(&_keyDevice);
	_itemList.push_back(&_medkit);
	_itemList.push_back(&_ladder);
	_itemList.push_back(&_rope);
	_itemList.push_back(&_key);
	_itemList.push_back(&_translator);
	_itemList.push_back(&_ale);
	_itemList.push_back(&_paper);
	_itemList.push_back(&_waldos);
	_itemList.push_back(&_stasisBox2);
	_itemList.push_back(&_ring);
	_itemList.push_back(&_cloak);
	_itemList.push_back(&_tunic);
	_itemList.push_back(&_candle);
	_itemList.push_back(&_straw);
	_itemList.push_back(&_scimitar);
	_itemList.push_back(&_sword);
	_itemList.push_back(&_helmet);
	_itemList.push_back(&_items);
	_itemList.push_back(&_concentrator);
	_itemList.push_back(&_nullifier);
	_itemList.push_back(&_peg);
	_itemList.push_back(&_vial);
	_itemList.push_back(&_jacket);
	_itemList.push_back(&_tunic2);
	_itemList.push_back(&_bone);
	_itemList.push_back(&_jar);
	_itemList.push_back(&_emptyJar);

	_selectedItem = NULL;
}

void InvObjectList::synchronise(Serialiser &s) {
	SYNC_POINTER(_selectedItem);

List<InvObject *> _itemList;
}

/*--------------------------------------------------------------------------*/

void EventHandler::dispatch() {  
	if (_action) _action->dispatch();
}

void EventHandler::setAction(Action *action, EventHandler *fmt, ...) {
	if (_action) {
		_action->_fmt = NULL;
		_action->remove();
	}

	_action = action;
	if (action) {
		va_list va;
		va_start(va, fmt);
		_action->attached(this, fmt, va);
		va_end(va);
	}
}

/*--------------------------------------------------------------------------*/

Action::Action() {
	_actionIndex = 0;
	_owner = NULL;
	_fmt = NULL;
}

void Action::synchronise(Serialiser &s) {
	EventHandler::synchronise(s);
	if (s.isLoading())
		remove();

	SYNC_POINTER(_owner);
	s.syncAsSint32LE(_actionIndex);
	s.syncAsSint32LE(_delayFrames);
	s.syncAsUint32LE(_startFrame);
	s.syncAsSint16LE(_field16);
	SYNC_POINTER(_fmt);
}

void Action::remove() {
	if (_action)
		_action->remove();

	if (_owner) {
		_owner->_action = NULL;
		_owner = NULL;
	} else {
		_globals->_sceneManager.removeAction(this);
	}

	_field16 = 0;
	if (_fmt)
		_fmt->signal();
}

void Action::process(Event &event) {
	if (_action)
		_action->process(event);
}

void Action::dispatch() {
	if (_action)
		_action->dispatch();

	if (_delayFrames) {
		uint32 frameNumber = _globals->_events.getFrameNumber();

		if (frameNumber >= _startFrame) {
			_delayFrames -= frameNumber - _startFrame;
			_startFrame = frameNumber;
			if (_delayFrames <= 0) {
				_delayFrames = 0;
				signal();
			}
		}
	}
}

void Action::attached(EventHandler *newOwner, EventHandler *fmt, va_list va) {
	_actionIndex = 0;
	_delayFrames = 0;
	_startFrame = _globals->_events.getFrameNumber();
	_owner = newOwner;
	_fmt = fmt;
	_field16 = 1;
	signal();
}

void Action::setDelay(int numFrames) {
	_delayFrames = numFrames;
	_startFrame = _globals->_events.getFrameNumber();
}

/*--------------------------------------------------------------------------*/

ObjectMover::~ObjectMover() {
	if (_sceneObject->_mover == this)
		_sceneObject->_mover = NULL;
}

void ObjectMover::synchronise(Serialiser &s) {
	EventHandler::synchronise(s);
	
	s.syncAsSint16LE(_destPosition.x); s.syncAsSint16LE(_destPosition.y);
	s.syncAsSint16LE(_moveDelta.x); s.syncAsSint16LE(_moveDelta.y);
	s.syncAsSint16LE(_moveSign.x); s.syncAsSint16LE(_moveSign.y);
	s.syncAsSint32LE(_minorDiff);
	s.syncAsSint32LE(_majorDiff);
	s.syncAsSint32LE(_field1A);
	SYNC_POINTER(_action);
	SYNC_POINTER(_sceneObject);
}

void ObjectMover::remove() {
	if (_sceneObject->_mover == this)
		_sceneObject->_mover = NULL;

	delete this;
}

void ObjectMover::dispatch() {
	Common::Point currPos = _sceneObject->_position;
	int yDiff = _sceneObject->_yDiff;

	if (dontMove())
		return;

	_sceneObject->_field6E = NULL;
	if (_moveDelta.x >= _moveDelta.y) {
		int xAmount = _moveSign.x * _sceneObject->_moveDiff.x * _sceneObject->_percent / 100;
		if (!xAmount)
			xAmount = _moveSign.x;
		currPos.x += xAmount;
		
		int yAmount = ABS(_destPosition.y - currPos.y);
		int yChange = _majorDiff / ABS(xAmount);
		int ySign;

		if (!yChange)
			ySign = _moveSign.y;
		else {
			int v = yAmount / yChange;
			_field1A += yAmount % yChange;
			if (_field1A >= yChange) {
				++v;
				_field1A -= yChange;
			}
			
			ySign = _moveSign.y * v;
		}

		currPos.y += ySign;
		_majorDiff -= ABS(xAmount);

	} else {
		int yAmount = _moveSign.y * _sceneObject->_moveDiff.y * _sceneObject->_percent / 100;
		if (!yAmount)
			yAmount = _moveSign.y;
		currPos.y += yAmount;
		
		int xAmount = ABS(_destPosition.x - currPos.x);
		int xChange = _majorDiff / ABS(yAmount);
		int xSign;

		if (!xChange)
			xSign = _moveSign.x;
		else {
			int v = xAmount / xChange;
			_field1A += xAmount % xChange;
			if (_field1A >= xChange) {
				++v;
				_field1A -= xChange;
			}
			
			xSign = _moveSign.x * v;
		}

		currPos.x += xSign;
		_majorDiff -= ABS(yAmount);
	}

//TODO:	_sceneObject->_field6E = _sceneObject->proc1(currPos);
	if (!_sceneObject->_field6E) {
		_sceneObject->setPosition(currPos, yDiff);
		_sceneObject->getHorizBounds();

		if (dontMove()) {
			_sceneObject->_position = _destPosition;
			endMove();
		}
	} else {
		endMove();
	}
}

void ObjectMover::setup(const Common::Point &destPos) {
	_sceneObject->calcAngle(destPos);

	if ((_sceneObject->_objectWrapper) && !(_sceneObject->_flags & OBJFLAG_8))
		_sceneObject->_objectWrapper->dispatch();

	// Get the difference
	int diffX = destPos.x - _sceneObject->_position.x;
	int diffY = destPos.y - _sceneObject->_position.y;
	int xSign = (diffX < 0) ? -1 : (diffX > 0 ? 1 : 0);
	int ySign = (diffY < 0) ? -1 : (diffY > 0 ? 1 : 0);
	diffX = ABS(diffX);
	diffY = ABS(diffY);
	
	if (diffX < diffY) {
		_minorDiff = diffX / 2;
		_majorDiff = diffY;
	} else {
		_minorDiff = diffY / 2;
		_majorDiff = diffX;
	}

	// Set the destination position
	_destPosition = destPos;
	_moveDelta = Common::Point(diffX, diffY);
	_moveSign = Common::Point(xSign, ySign);
	_field1A = 0;

	if (!diffX && !diffY)
		// Object is already at the correct destination
		endMove();
}

bool ObjectMover::dontMove() const {
	return (_majorDiff <= 0);
}

void ObjectMover::endMove() {
	EventHandler *actionP = _action;
	remove();

	if (actionP)
		actionP->signal();
}

/*--------------------------------------------------------------------------*/

ObjectMover2::ObjectMover2(): ObjectMover() {
	_destObject = NULL;
}

void ObjectMover2::synchronise(Serialiser &s) {
	ObjectMover::synchronise(s);

	SYNC_POINTER(_destObject);
	s.syncAsSint32LE(_minArea);
	s.syncAsSint32LE(_maxArea);
}

void ObjectMover2::dispatch() {
	int area = _sceneObject->getSpliceArea(_destObject);
	if (area > _maxArea) {
		// Setup again for the new destination
		setup(_destObject->_position);
	} else if (area >= _minArea) {
		// Keep dispatching
		ObjectMover::dispatch();
	} else {
		// Within minimum, so end move
		endMove();
	}
}

void ObjectMover2::startMove(SceneObject *sceneObj, va_list va) {
	// Set up fields
	_sceneObject = sceneObj;

	_minArea = va_arg(va, int);
	_maxArea = va_arg(va, int);
	_destObject = va_arg(va, SceneObject *);

	setup(_destObject->_position);
}

void ObjectMover2::endMove() {
	_sceneObject->_field6E = 64;
}

/*--------------------------------------------------------------------------*/

void ObjectMover3::dispatch() {
	int area = _sceneObject->getSpliceArea(_destObject);
	if (area <= _minArea) {
		endMove();
	} else {
		setup(_destObject->_position);
		ObjectMover::dispatch();
	}
}

void ObjectMover3::startMove(SceneObject *sceneObj, va_list va) {
	_sceneObject = va_arg(va, SceneObject *);
	_destObject = va_arg(va, SceneObject *);
	_minArea = va_arg(va, int);
	_action = va_arg(va, Action *);

	setup(_destObject->_position);
}

void ObjectMover3::endMove() {
	ObjectMover::endMove();
}

/*--------------------------------------------------------------------------*/

void NpcMover::startMove(SceneObject *sceneObj, va_list va) {
	_sceneObject = sceneObj;

	Common::Point *destPos = va_arg(va, Common::Point *);
	_action = va_arg(va, Action *);

	setup(*destPos);
}

/*--------------------------------------------------------------------------*/

void PlayerMover::synchronise(Serialiser &s) {
	NpcMover::synchronise(s);

	s.syncAsSint16LE(_finalDest.x); s.syncAsSint16LE(_finalDest.y);
	s.syncAsSint32LE(_routeIndex);

	for (int i = 0; i < MAX_ROUTE_SIZE; ++i) {
		s.syncAsSint16LE(_routeList[i].x); s.syncAsSint16LE(_routeList[i].y);
	}
}

void PlayerMover::startMove(SceneObject *sceneObj, va_list va) {
	_sceneObject = sceneObj;
	Common::Point *pt = va_arg(va, Common::Point *);
	_finalDest = *pt;
	_action = va_arg(va, Action *);
	
	setDest(_finalDest);
}

void PlayerMover::endMove() {
	while (++_routeIndex != 0) {
		if ((_routeList[_routeIndex].x == ROUTE_END_VAL) ||
			(_routeList[_routeIndex].y == ROUTE_END_VAL) ||
			(_sceneObject->_field6E)) {
			// Movement route is completely finished
			ObjectMover::endMove();
			return;
		}

		if ((_routeList[_routeIndex].x != _sceneObject->_position.x) ||
			(_routeList[_routeIndex].y != _sceneObject->_position.y))
			break;
	}

	// Set up the new interim destination along the route
	_globals->_walkRegions._routeEnds.moveSrc = _globals->_walkRegions._routeEnds.moveDest;
	_globals->_walkRegions._routeEnds.moveDest = _routeList[_routeIndex];
	setup(_routeList[_routeIndex]);
	dispatch();
}

void PlayerMover::setDest(const Common::Point &destPos) {
	_routeList[0] = _sceneObject->_position;

	if (_globals->_walkRegions._resNum == -1) {
		// Scene has no walk regions defined, so player can walk anywhere directly
		_routeList[0] = destPos;
		_routeList[1] = Common::Point(ROUTE_END_VAL, ROUTE_END_VAL);
	} else {
		// Figure out a path to the destination (or as close as possible to it)
		pathfind(_routeList, _sceneObject->_position, destPos, _globals->_walkRegions._routeEnds);
	}

	_routeIndex = 0;
	_globals->_walkRegions._routeEnds.moveSrc = _sceneObject->_position;
	_globals->_walkRegions._routeEnds.moveDest = _routeList[0];
	setup(_routeList[0]);
}

#define REGION_LIST_SIZE 20

void PlayerMover::pathfind(Common::Point *routeList, Common::Point srcPos, Common::Point destPos, RouteEnds routeEnds) {
	List<int> regionIndexes;
	RouteEnds tempRouteEnds;
	int routeRegions[REGION_LIST_SIZE];
	Common::Point objPos;

	// Get the region the source is in
	int srcRegion = _globals->_walkRegions.indexOf(srcPos);
	if (srcRegion == -1) {
		srcRegion = findClosestRegion(srcPos, regionIndexes);
	}

	// Main loop for building up the path
	routeRegions[0] = 0;
	while (!routeRegions[0]) {
		// Check the destination region
		int destRegion = _globals->_walkRegions.indexOf(destPos, &regionIndexes);

		if ((srcRegion == -1) && (destRegion == -1)) {
			// Both source and destination are outside walkable areas
		} else if (srcRegion == -1) {
			// Source is outside walkable areas
			tempRouteEnds = routeEnds;
			objPos = _sceneObject->_position;
			
			Common::Point newPos;
			findLinePoint(&tempRouteEnds, &objPos, 1, &newPos);
			int srcId = _globals->_walkRegions.indexOf(newPos);

			if (srcId == -1) {
				tempRouteEnds.moveDest = tempRouteEnds.moveSrc;
				tempRouteEnds.moveSrc = routeEnds.moveDest;

				findLinePoint(&tempRouteEnds, &objPos, 1, &newPos);
				srcRegion = _globals->_walkRegions.indexOf(newPos);

				if (srcRegion == -1)
					srcRegion = checkMover(srcPos, destPos);
			}

		} else if (destRegion == -1) {
			// Destination is outside walkable areas
			destRegion = findClosestRegion(destPos, regionIndexes);
			if (destRegion == -1) {
				// No further route found, so end it
				*routeList++ = srcPos;
				break;
			} else {
				_finalDest = destPos;
			}
		}

		if (srcRegion == destRegion) {
			*routeList++ = (srcRegion == -1) ? srcPos : destPos;
			break;
		}

		int var6;
		proc1(routeRegions, srcRegion, destRegion, var6);

		if (!routeRegions[0]) {
			regionIndexes.push_back(destRegion);
			continue;
		}

		_globals->_walkRegions._field18[0]._pt1 = srcPos;
		_globals->_walkRegions._field18[0]._pt2 = srcPos;
		_globals->_walkRegions._field18[1]._pt1 = destPos;
		_globals->_walkRegions._field18[1]._pt2 = destPos;

		int tempList[REGION_LIST_SIZE];
		tempList[0] = 0;
		int endIndex = 0;
		int idx = 1;
	
		do {
			int breakEntry = routeRegions[idx];
			int breakEntry2 = routeRegions[idx + 1];

			int listIndex = 0; 
			while (_globals->_walkRegions._idxList[_globals->_walkRegions[breakEntry]._idxListIndex + listIndex] !=
					breakEntry2)
				++listIndex;

			tempList[idx] = _globals->_walkRegions._idxList2[_globals->_walkRegions[breakEntry]._idxList2Index 
				+ listIndex];
			
			++endIndex;
		} while (routeRegions[++idx] != destRegion);

		tempList[idx] = 1;
		idx = 0;
		for (int listIndex = 1; listIndex <= endIndex; ++listIndex) {
			int var10 = tempList[listIndex];
			int var12 = tempList[listIndex + 1];

			if (sub_F8E5(_globals->_walkRegions._field18[0]._pt1, _globals->_walkRegions._field18[var12]._pt1,
					_globals->_walkRegions._field18[var10]._pt1, _globals->_walkRegions._field18[var10]._pt2) &&
				sub_F8E5(_globals->_walkRegions._field18[0]._pt1, _globals->_walkRegions._field18[var12]._pt2,
					_globals->_walkRegions._field18[var10]._pt1, _globals->_walkRegions._field18[var10]._pt2))
				continue;

			Common::Point tempPt;
			if (sub_F8E5(_globals->_walkRegions._field18[0]._pt1, _globals->_walkRegions._field18[0]._pt1,
					_globals->_walkRegions._field18[var10]._pt1, _globals->_walkRegions._field18[var10]._pt2, &tempPt)) {
				// Add point to the route list
				_globals->_walkRegions._field18[0]._pt1 = tempPt;
				*routeList++ = tempPt;
			} else {
				int v16 = 
					(findDistance(_globals->_walkRegions._field18[0]._pt1, _globals->_walkRegions._field18[var10]._pt1) << 1) +
					(findDistance(_globals->_walkRegions._field18[var10]._pt1, _globals->_walkRegions._field18[1]._pt1) << 1) +
					findDistance(_globals->_walkRegions._field18[var10]._pt1, _globals->_walkRegions._field18[var12]._pt1) +
					findDistance(_globals->_walkRegions._field18[var10]._pt1, _globals->_walkRegions._field18[var12]._pt2);
				
				int v1A = 
					(findDistance(_globals->_walkRegions._field18[0]._pt1, _globals->_walkRegions._field18[var10]._pt2) << 1) +
					(findDistance(_globals->_walkRegions._field18[var10]._pt2, _globals->_walkRegions._field18[1]._pt2) << 1) +
					findDistance(_globals->_walkRegions._field18[var10]._pt2, _globals->_walkRegions._field18[var12]._pt1) +
					findDistance(_globals->_walkRegions._field18[var10]._pt2, _globals->_walkRegions._field18[var12]._pt2);

				if (v16 < v1A) {
					checkMovement2(_globals->_walkRegions._field18[var10]._pt1, 
						_globals->_walkRegions._field18[var10]._pt2, 1, objPos);
				} else {
					checkMovement2(_globals->_walkRegions._field18[var10]._pt2, 
						_globals->_walkRegions._field18[var10]._pt1, 1, objPos);
				}

				*routeList++ = objPos;
			}
		}

		// Add in the route entry
		*routeList++ = _globals->_walkRegions._field18[1]._pt1;
	}

	// Mark the end of the path
	*routeList = Common::Point(ROUTE_END_VAL, ROUTE_END_VAL);
}

int PlayerMover::regionIndexOf(const Common::Point &pt) {
	for (uint idx = 0; idx < _globals->_walkRegions._regionList.size(); ++idx) {
		if (_globals->_walkRegions._regionList[idx].contains(pt))
			return idx + 1;
	}

	return 0;
}

int PlayerMover::findClosestRegion(Common::Point &pt, List<int> &indexList) {
	int newY = pt.y;
	int result = 0;

	for (int idx = 1; idx < SCREEN_WIDTH; ++idx, newY += idx) {
		int newX = pt.x + idx;
		result = regionIndexOf(newX, pt.y);

		if ((result == 0) || indexList.contains(result)) {
			newY = pt.y + idx;
			result = regionIndexOf(newX, newY);

			if ((result == 0) || indexList.contains(result)) {
				newX -= idx;
				result = regionIndexOf(newX, newY);

				if ((result == 0) || indexList.contains(result)) {
					newX -= idx;
					result = regionIndexOf(newX, newY);

					if ((result == 0) || indexList.contains(result)) {
						newY -= idx;
						result = regionIndexOf(newX, newY);

						if ((result == 0) || indexList.contains(result)) {
							newY -= idx;
							result = regionIndexOf(newX, newY);

							if ((result == 0) || indexList.contains(result)) {
								newX += idx;
								result = regionIndexOf(newX, newY);

								if ((result == 0) || indexList.contains(result)) {
									newX += idx;
									result = regionIndexOf(newX, newY);

									if ((result == 0) || indexList.contains(result)) {
										continue;
									}
								}
							}
						}
					}
				}
			}
		}

		// Found an index
		pt.x = newX;
		pt.y = newY;
		return result;
	}

	return (result == 0) ? -1 : result;
}

Common::Point *PlayerMover::findLinePoint(RouteEnds *routeEnds, Common::Point *objPos, int length, Common::Point *outPos) {
	int xp = objPos->x + (((routeEnds->moveDest.y - routeEnds->moveSrc.y) * 9) / 8);
	int yp = objPos->y - (((routeEnds->moveDest.x - routeEnds->moveSrc.x) * 8) / 9);

	int xDiff = xp - objPos->x;
	int yDiff = yp - objPos->y;
	int xDirection = (xDiff == 0) ? 0 : ((xDiff < 0) ? 1 : -1);
	int yDirection = (yDiff == 0) ? 0 : ((yDiff < 0) ? 1 : -1);
	xDiff = ABS(xDiff);
	yDiff = ABS(yDiff);
	int majorChange = MAX(xDiff, yDiff) / 2;

	int outX = objPos->x;
	int outY = objPos->y;

	while (length-- > 0) {
		if (xDiff < yDiff) {
			outY += yDirection;
			majorChange += xDiff;
			if (majorChange > yDiff) {
				majorChange -= yDiff;
				outX += xDirection;
			}
		} else {
			outX += xDirection;
			majorChange += yDiff;
			if (majorChange > xDiff) {
				majorChange -= xDiff;
				outY += yDirection;
			}
		}
	}

	outPos->x = outX;
	outPos->y = outY;
	return outPos;
}

int PlayerMover::checkMover(Common::Point &srcPos, const Common::Point &destPos) {
	int regionIndex = 0;
	Common::Point objPos = _sceneObject->_position;
	uint32 regionBitList = _sceneObject->_regionBitList;
	_sceneObject->_regionBitList = 0;

	_sceneObject->_position.x = srcPos.x;
	_sceneObject->_position.y = srcPos.y;
	_sceneObject->_mover = NULL;

	NpcMover *mover = new NpcMover();
	_sceneObject->addMover(mover, &destPos, NULL);
	
	// Handle automatic movement of the player until a walkable region is reached,
	// or the end point of the movement is
	do {
		_sceneObject->_mover->dispatch();

		// Scan walk regions for point
		for (uint idx = 0; idx < _globals->_walkRegions._regionList.size(); ++idx) {
			if (_globals->_walkRegions[idx].contains(_sceneObject->_position)) {
				regionIndex = idx + 1;
				srcPos = _sceneObject->_position;
				break;
			}
		}
	} while ((regionIndex == 0) && (_sceneObject->_mover) && !_vm->shouldQuit());

	_sceneObject->_position = objPos;
	_sceneObject->_regionBitList = regionBitList;

	if (_sceneObject->_mover)
		_sceneObject->_mover->remove();

	_sceneObject->_mover = this;
	return regionIndex;
}

void PlayerMover::checkMovement2(const Common::Point &srcPos, const Common::Point &destPos, int numSteps, Common::Point &ptOut) {
	Common::Point objPos = _sceneObject->_position;
	_sceneObject->_position = srcPos;
	uint32 regionBitList = _sceneObject->_regionBitList;
	_sceneObject->_position = srcPos;
	_sceneObject->_mover = NULL;

	NpcMover *mover = new NpcMover();
	_sceneObject->addMover(mover, &destPos, NULL);

	while ((numSteps > 0) && ((_sceneObject->_position.x != destPos.x) || (_sceneObject->_position.y != destPos.y))) {
		_sceneObject->_mover->dispatch();
		--numSteps;
	}

	ptOut = _sceneObject->_position;
	_sceneObject->_position = objPos;
	_sceneObject->_regionBitList = regionBitList;

	if (_sceneObject->_mover)
		_sceneObject->_mover->remove();
	
	_sceneObject->_mover = this;
}

int PlayerMover::proc1(int *routeList, int srcRegion, int destRegion, int &v) {
	int tempList[REGION_LIST_SIZE + 1];
	v = 0;
	for (int idx = 0; idx <= *routeList; ++idx)
		tempList[idx] = routeList[idx];

	if (*routeList == REGION_LIST_SIZE)
		// Sequence too long
		return 32000;

	int regionIndex;
	for (regionIndex = 1; regionIndex <= *tempList; ++regionIndex) {
		if (routeList[regionIndex] == srcRegion)
			// Current path returns to original source region, so don't allow it
			return 32000;
	}

	WalkRegion &srcWalkRegion = _globals->_walkRegions[srcRegion];
	int distance;
	if (!routeList[0]) {
		// No route 
		distance = 0;
	} else {
		WalkRegion &region = _globals->_walkRegions[routeList[*routeList]];
		distance = findDistance(srcWalkRegion._pt, region._pt);
	}

	tempList[++*tempList] = srcRegion;
	int newIndex = *tempList;

	if (srcRegion == destRegion) {
		v = 1;
		for (int idx = newIndex; idx <= *tempList; ++idx) {
			routeList[idx] = tempList[idx];
			++*routeList;
		}
		return distance;
	} else {
		int foundIndex = 0;
		int idx = 0; 
		int currDest;
		while ((currDest = _globals->_walkRegions._idxList[srcWalkRegion._idxListIndex + idx]) != 0) {
			if (currDest == destRegion) {
				foundIndex = idx;
				break;
			}

			++idx;
		}

		int resultOffset = 31990;
		while (((currDest = _globals->_walkRegions._idxList[srcWalkRegion._idxListIndex + foundIndex]) != 0) && (v == 0)) {
			int newDistance = proc1(tempList, currDest, destRegion, v);
			
			if ((newDistance <= resultOffset) || v) {
				routeList[0] = newIndex - 1;
				
				for (int i = newIndex; i <= tempList[0]; ++i) {
					routeList[i] = tempList[i];
					++routeList[0];
				}

				resultOffset = newDistance;
			}

			tempList[0] = newIndex;
			++foundIndex;
		}

		v = 0;
		return resultOffset + distance;
	}
}

int PlayerMover::findDistance(const Common::Point &pt1, const Common::Point &pt2) {
	int diff = ABS(pt1.x - pt2.x);
	double xx = diff * diff;
	diff = ABS(pt1.y - pt2.y);
	double yy = diff * 8.0 / 7.0;
	yy *= yy;

	return (int)sqrtf(xx + yy);
}

bool PlayerMover::sub_F8E5(const Common::Point &pt1, const Common::Point &pt2, const Common::Point &pt3,
						  const Common::Point &pt4, Common::Point *ptOut) {
	double diff1 = pt2.x - pt1.x;
	double diff2 = pt2.y - pt1.y;
	double diff3 = pt4.x - pt3.x;
	double diff4 = pt4.y - pt3.y;
	double var10 = 0.0, var8 = 0.0;
	double var18 = 0.0, var20 = 0.0;

	if (diff1 != 0.0) {
		var8 = diff2 / diff1;
		var18 = pt1.y - (pt1.x * var8);
	}
	if (diff3 != 0.0) {
		var10 = diff4 / diff3;
		var20 = pt3.y - (pt3.x * var10);
	}

	if (var8 == var10)
		return false;
	
	double var48, var50;
	if (diff1 == 0) {
		if (diff3 == 0)
			return false;

		var48 = pt1.x;
		var50 = var10 * var48 + var20;
	} else {
		var48 = (diff3 == 0) ? pt3.x : (var20 - var18) / (var8 - var10);
		var50 = var8 * var48 + var18;
	}

	bool var52 = false, var56 = false, var54 = false, var58 = false;
	Common::Point tempPt((int)(var48 + 0.5), (int)(var50 + 0.5));

	if ((tempPt.x >= pt3.x) && (tempPt.x <= pt4.x))
		var56 = true;
	else if ((tempPt.x >= pt4.x) && (tempPt.x <= pt3.x))
		var56 = true;
	if (var56) {
		if ((tempPt.y >= pt3.y) && (tempPt.y <= pt4.y))
			var58 = true;
		else if ((tempPt.y >= pt4.y) && (tempPt.y <= pt3.y))
			var58 = true;
	}

	if ((tempPt.x >= pt1.x) && (tempPt.x <= pt2.x))
		var52 = true;
	else if ((tempPt.x >= pt2.x) && (tempPt.x <= pt1.x))
		var52 = true;
	if (var52) {
		if ((tempPt.y >= pt1.y) && (tempPt.y <= pt2.y))
			var54 = true;
		else if ((tempPt.y >= pt2.y) && (tempPt.y <= pt1.y))
			var54 = true;
	}

	if (var52 && var54 && var56 && var58) {
		if (ptOut)
			*ptOut = tempPt;
		return true;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

PaletteModifier::PaletteModifier() {
	_scenePalette = NULL;
	_action = NULL;
}

/*--------------------------------------------------------------------------*/

PaletteRotation::PaletteRotation(): PaletteModifier() {
	_disabled = false;
	_delayFrames = 0;
	_delayCtr = 0;
	_frameNumber = _globals->_events.getFrameNumber();
}

void PaletteRotation::synchronise(Serialiser &s) {
	PaletteModifier::synchronise(s);

	s.syncAsByte(_disabled);
	s.syncAsSint32LE(_delayFrames);
	s.syncAsSint32LE(_delayCtr);
	s.syncAsUint32LE(_frameNumber);
	s.syncAsSint32LE(_currIndex);
	s.syncAsSint32LE(_start);
	s.syncAsSint32LE(_end);
	s.syncAsSint32LE(_rotationMode);
	s.syncAsSint32LE(_duration);
	for (int i = 0; i < 256; ++i)
		s.syncAsUint32LE(_palette[i]);
}

void PaletteRotation::signal() {
	if (_delayCtr) {
		uint32 frameNumber = _globals->_events.getFrameNumber();

		if (frameNumber >= _frameNumber) {
			_delayCtr -= frameNumber - _frameNumber;
			_frameNumber = frameNumber;

			if (_delayCtr < 0)
				_delayCtr = 0;
		}
	}

	if (_delayCtr)
		return;
	_delayCtr = _delayFrames;
	if (_disabled)
		return;

	bool flag = true;
	switch (_rotationMode) {
	case -1:
		if (--_currIndex < _start) {
			flag = decDuration();
			if (flag)
				_currIndex = _end - 1;
		}
		break;
	case 1:
		if (++_currIndex >= _end) {
			flag = decDuration();
			if (flag)
				_currIndex = _start;
		}
		break;
	case 2:
		if (++_currIndex >= _end) {
			flag = decDuration();
			if (flag) {
				_currIndex = _end - 2;
				_rotationMode = 3;
			}
		}
		break;
	case 3:
		if (--_currIndex < _start) {
			flag = decDuration();
			if (flag) {
				_currIndex = _start + 1;
				_rotationMode = 2;
			}
		}
		break;
	}

	if (flag) {
		int count2 = _currIndex - _start;
		int count = _end - _currIndex;
		g_system->getPaletteManager()->setPalette((const byte *)&_palette[_currIndex], _start, count);

		if (count2) {
			g_system->getPaletteManager()->setPalette((const byte *)&_palette[_start], _start, count2);
		}
	}
}

void PaletteRotation::remove() {
	Action *action = _action;
	g_system->getPaletteManager()->setPalette((const byte *)&_palette[0], _start, _end - _start);

	if (_scenePalette->_listeners.contains(this))
		_scenePalette->_listeners.remove(this);

	delete this;
	if (action)
		action->signal();
}

void PaletteRotation::set(ScenePalette *palette, int start, int end, int rotationMode, int duration, Action *action) {
	_duration = duration;
	_disabled = false;
	_action = action;
	_scenePalette = palette;

	Common::copy(&palette->_palette[0], &palette->_palette[256], &_palette[0]);

	_start = start;
	_end = end + 1;
	_rotationMode = rotationMode;

	switch (_rotationMode) {
	case -1:
	case 3:
		_currIndex = _end;
		break;
	default:
		_currIndex = _start;
		break;
	}		
}

void PaletteRotation::setPalette(ScenePalette *palette, bool disabled) {
	_scenePalette = palette;
	_disabled = disabled;
	_delayFrames = 100;
}

bool PaletteRotation::decDuration() {
	if (_duration) {
		if (--_duration == 0) {
			remove();
			return false;
		}
	}
	return true;
}

void PaletteRotation::setDelay(int amount) {
	_delayFrames = _delayCtr = amount;
}

/*--------------------------------------------------------------------------*/

ScenePalette::ScenePalette() { 
	// Set a default gradiant range
	for (int idx = 0; idx < 256; ++idx)
		_palette[idx] = idx | (idx << 8) | (idx << 16);

	_field412 = 0;
}

ScenePalette::ScenePalette(int paletteNum) { 
	loadPalette(paletteNum); 
}

bool ScenePalette::loadPalette(int paletteNum) {
	byte *palData = _vm->_dataManager->getResource(RES_PALETTE, paletteNum, 0, true);
	if (!palData)
		return false;

	int palStart = READ_LE_UINT16(palData);
	int palSize = READ_LE_UINT16(palData + 2);
	assert(palSize <= 256);

	uint32 *destP = &_palette[palStart];
	byte *srcP = palData + 6;


	for (int i = 0; i < palSize; ++i, srcP += 3, ++destP)
		*destP = *srcP | (*(srcP + 1) << 8) | (*(srcP + 2) << 16);

	DEALLOCATE(palData);
	return true;
}

void ScenePalette::refresh() {
	// Set indexes for standard colours to closest colour in the palette
	_colours.background = indexOf(255, 255, 255);	// White background
	_colours.foreground = indexOf(0, 0, 0);			// Black foreground
	_redColour = indexOf(180, 0, 0);				// Red-ish
	_greenColour = indexOf(0, 180, 0);				// Green-ish
	_blueColour = indexOf(0, 0, 180);				// Blue-ish
	_aquaColour = indexOf(0, 180, 180);				// Aqua
	_purpleColour = indexOf(180, 0, 180);			// Purple
	_limeColour = indexOf(180, 180, 0);				// Lime

	// Refresh the palette
	g_system->getPaletteManager()->setPalette((const byte *)&_palette[0], 0, 256);
}

/**
 * Loads a section of the palette into the game palette
 */
void ScenePalette::setPalette(int index, int count) {
	g_system->getPaletteManager()->setPalette((const byte *)&_palette[index], index, count);
}

/**
 * Returns the palette index with the closest matching colour to that specified
 * @param r			R component
 * @param g			G component
 * @param b			B component
 * @param threshold	Closeness threshold.
 * @remarks	A threshold may be provided to specify how close the matching colour must be
 */
uint8 ScenePalette::indexOf(uint r, uint g, uint b, int threshold) {
	int palIndex = -1;

	for (int i = 0; i < 256; ++i) {
		int ir = _palette[i] & 0xff;
		int ig = (_palette[i] >> 8) & 0xff;
		int ib = (_palette[i] >> 16) & 0xff;
		int rDiff = abs(ir - (int)r);
		int gDiff = abs(ig - (int)g);
		int bDiff = abs(ib - (int)b);
		
		int idxThreshold = rDiff * rDiff + gDiff * gDiff + bDiff * bDiff;
		if (idxThreshold <= threshold) {
			threshold = idxThreshold;
			palIndex = i;
		}
	}

	return palIndex;
}

/**
 * Loads the specified range of the palette with the current system palette
 * @param start		Start index
 * @param count		Number of palette entries
 */
void ScenePalette::getPalette(int start, int count) {
	g_system->getPaletteManager()->grabPalette((byte *)&_palette[start], start, count);
}

void ScenePalette::signalListeners() {
	for (List<PaletteModifier *>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->signal();
	}
}

void ScenePalette::clearListeners() {
	List<PaletteModifier *>::iterator i = _listeners.begin();
	while (i != _listeners.end()) {
		PaletteModifier *obj = *i;
		++i;
		obj->remove();
	}
}

void ScenePalette::fade(const byte *adjustData, bool fullAdjust, int percent) {
	uint32 tempPalette[256];

	// Ensure the percent adjustment is within 0 - 100%
	percent = CLIP(percent, 0, 100);

	for (int palIndex = 0; palIndex < 256; ++palIndex) {
		const byte *srcP = (const byte *)&_palette[palIndex];
		byte *destP = (byte *)&tempPalette[palIndex];

		for (int rgbIndex = 0; rgbIndex < 3; ++rgbIndex, ++srcP, ++destP) {
			*destP = *srcP - ((*srcP - adjustData[rgbIndex]) * (100 - percent)) / 100;
		}

		if (fullAdjust)
			adjustData += 3;
	}

	// Set the altered pale4tte
	g_system->getPaletteManager()->setPalette((const byte *)&tempPalette[0], 0, 256);
	g_system->updateScreen();
}

PaletteRotation *ScenePalette::addRotation(int start, int end, int rotationMode, int duration, Action *action) {
	PaletteRotation *obj = new PaletteRotation();

	if ((rotationMode == 2) || (rotationMode == 3))
		duration <<= 1;

	obj->set(this, start, end, rotationMode, duration, action);
	_listeners.push_back(obj);
	return obj;
}

void ScenePalette::changeBackground(const Rect &bounds, FadeMode fadeMode) {
	ScenePalette tempPalette;
	if (_globals->_sceneManager._hasPalette) {
		if ((fadeMode == FADEMODE_GRADUAL) || (fadeMode == FADEMODE_IMMEDIATE)) {
			// Fade out any active palette
			tempPalette.getPalette();
			uint32 adjustData = 0;

			for (int percent = 100; percent >= 0; percent -= 5) {
				if (fadeMode == FADEMODE_IMMEDIATE)
					percent = 0;
				tempPalette.fade((byte *)&adjustData, false, percent);
				g_system->delayMillis(10);
			}
		} else {
			_globals->_scenePalette.refresh();
			_globals->_sceneManager._hasPalette = false;
		}
	}

	_globals->_screenSurface.copyFrom(_globals->_sceneManager._scene->_backSurface, 
		bounds, Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), NULL);
	tempPalette._listeners.clear2();
}

void ScenePalette::synchronise(Serialiser &s) {
	for (int i = 0; i < 256; ++i)
		s.syncAsUint32LE(_palette[i]);
	s.syncAsSint32LE(_colours.foreground);
	s.syncAsSint32LE(_colours.background);

	s.syncAsSint32LE(_field412);
	s.syncAsByte(_redColour);
	s.syncAsByte(_greenColour);
	s.syncAsByte(_blueColour);
	s.syncAsByte(_aquaColour);
	s.syncAsByte(_purpleColour);
	s.syncAsByte(_limeColour);	
}

/*--------------------------------------------------------------------------*/

void SceneItem::synchronise(Serialiser &s) {
	EventHandler::synchronise(s);

	_bounds.synchronise(s);
	s.syncString(_msg);
	s.syncAsSint32LE(_fieldE);
	s.syncAsSint32LE(_field10);
	s.syncAsSint16LE(_position.x); s.syncAsSint32LE(_position.y);
	s.syncAsSint16LE(_yDiff);
	s.syncAsSint32LE(_sceneRegionId);
}

void SceneItem::remove() {
	_globals->_sceneItems.remove(this);
}

void SceneItem::doAction(int action) {
	const char *msg = NULL;

	switch ((int)action) {
	case CURSOR_LOOK:
		msg = LOOK_SCENE_HOTSPOT;
		break;
	case CURSOR_USE:
		msg = USE_SCENE_HOTSPOT;
		break;
	case CURSOR_TALK:
		msg = TALK_SCENE_HOTSPOT;
		break;
	case 0x1000:
		msg = SPECIAL_SCENE_HOTSPOT;
		break;
	default:
		msg = DEFAULT_SCENE_HOTSPOT;
		break;
	}
	
	GUIErrorMessage(msg);
}

bool SceneItem::contains(const Common::Point &pt) {
	const Rect &sceneBounds = _globals->_sceneManager._scene->_sceneBounds;

	if (_sceneRegionId == 0)
		return _bounds.contains(pt.x + sceneBounds.left, pt.y + sceneBounds.top);
	else 
		return _globals->_sceneRegions.indexOf(Common::Point(pt.x + sceneBounds.left, 
			pt.y + sceneBounds.top)) == _sceneRegionId;
}

void SceneItem::display(int resNum, int lineNum, ...) {
	Common::String msg = !resNum ? Common::String() : _vm->_dataManager->getMessage(resNum, lineNum);

	if (_globals->_sceneObjects->contains(&_globals->_sceneText)) {
		_globals->_sceneText.remove();
		_globals->_sceneObjects->draw();
	}

	GfxFontBackup font;
	Common::Point pos(160, 100);
	Rect textRect;
	int maxWidth = 120;
	bool keepOnscreen = false;
	bool centreText = true;
	
	if (resNum) {
		va_list va;
		va_start(va, lineNum);

		int mode;
		do {
			// Get next instruction
			mode = va_arg(va, int);

			switch (mode) {
			case SET_WIDTH:
				// Set width
				maxWidth = va_arg(va, int);
				_globals->_sceneText._width = maxWidth;
				break;
			case SET_X:
				// Set the X Position
				pos.x = va_arg(va, int);
				break;
			case SET_Y:
				// Set the Y Position
				pos.y = va_arg(va, int);
				break;
			case SET_FONT:
				// Set the font number
				_globals->_sceneText._fontNumber = va_arg(va, int);
				_globals->gfxManager()._font.setFontNumber(_globals->_sceneText._fontNumber);
				break;
			case SET_BG_COLOUR: {
				// Set the background colour
				int bgColour = va_arg(va, int);
				_globals->gfxManager()._font._colours.background = bgColour;
				if (!bgColour)
					_globals->gfxManager().setFillFlag(false);
				break;
			}
			case SET_FG_COLOUR:
				// Set the foreground colour
				_globals->_sceneText._colour1 = va_arg(va, int);
				_globals->gfxManager()._font._colours.foreground = _globals->_sceneText._colour1;
				break;
			case SET_KEEP_ONSCREEN:
				// Suppresses immediate display
				keepOnscreen = va_arg(va, int) != 0;
				break;
			case SET_EXT_BGCOLOUR: {
				// Set secondary bg colour
				int v = va_arg(va, int);
				_globals->_sceneText._colour2 = v;
				_globals->gfxManager()._font._colours2.background = v;
				break;
			}
			case SET_EXT_FGCOLOUR: {
				// Set secondary fg colour
				int v = va_arg(va, int);
				_globals->_sceneText._colour3 = v;
				_globals->gfxManager()._font._colours.foreground = v;
				break;
			}
			case SET_POS_MODE:
				// Set whether a custom x/y is used
				centreText = va_arg(va, int) != 0;
				break;
			case SET_TEXT_MODE:
				// Set the text mode
				_globals->_sceneText._textMode = (TextAlign)va_arg(va, int);
				break;
			default:
				break;
			}
		} while (mode != LIST_END);

		va_end(va);
	}
	
	if (resNum) {
		// Get required bounding size
		_globals->gfxManager().getStringBounds(msg.c_str(), textRect, maxWidth);
		textRect.centre(pos.x, pos.y);

		textRect.contain(_globals->gfxManager()._bounds);
		if (centreText) {
			_globals->_sceneText._colour1 = _globals->_sceneText._colour2;
			_globals->_sceneText._colour2 = 0;
			_globals->_sceneText._colour3 = 0;
		}

		_globals->_sceneText.setup(msg);
		if (centreText) {
			_globals->_sceneText.setPosition(Common::Point(
				_globals->_sceneManager._scene->_sceneBounds.left + textRect.left,
				_globals->_sceneManager._scene->_sceneBounds.top + textRect.top), 0);
		} else {
			_globals->_sceneText.setPosition(pos, 0);
		}

		_globals->_sceneText.setPriority2(255);
		_globals->_sceneObjects->draw();
	}

	// Unless the flag is set to keep the message on-screen, show it until a mouse or keypress, then remove it
	if (!keepOnscreen && !msg.empty()) {
		Event event;

		// Keep event on-screen until a mouse or keypress
		while (!_vm->getEventManager()->shouldQuit() && !_globals->_events.getEvent(event,
				EVENT_BUTTON_DOWN | EVENT_KEYPRESS)) {
			g_system->updateScreen();
			g_system->delayMillis(10);
		}

		_globals->_sceneText.remove();
	}
}

/*--------------------------------------------------------------------------*/

void SceneHotspot::doAction(int action) {
	switch ((int)action) {
	case CURSOR_LOOK:
		display(1, 0, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
		break;
	case CURSOR_USE:
		display(1, 5, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
		break;
	case CURSOR_TALK:
		display(1, 15, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
		break;
	case CURSOR_WALK:
		break;
	default:
		display(2, action, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void SceneObjectWrapper::setSceneObject(SceneObject *so) {
	_sceneObject = so;
	so->_strip = 1;
	so->_flags |= OBJFLAG_PANES;
}

void SceneObjectWrapper::synchronise(Serialiser &s) {
	EventHandler::synchronise(s);
	SYNC_POINTER(_sceneObject);
}

void SceneObjectWrapper::dispatch() {
	_visageImages.setVisage(_sceneObject->_visage);
	int frameCount = _visageImages.getFrameCount();
	int angle = _sceneObject->_angle;
	int strip = _sceneObject->_strip;

	if (frameCount == 4) {
		if ((angle > 314) || (angle < 45))
			strip = 4;
		if ((angle > 44) && (angle < 135))
			strip = 1;
		if ((angle >= 135) && (angle < 225))
			strip = 3;
		if ((angle >= 225) && (angle < 315))
			strip = 2;
	} else if (frameCount == 8) {
		if ((angle > 330) || (angle < 30))
			strip = 4;
		if ((angle >= 30) && (angle < 70))
			strip = 7;
		if ((angle >= 70) && (angle < 110))
			strip = 1;
		if ((angle >= 110) && (angle < 150))
			strip = 5;
		if ((angle >= 150) && (angle < 210))
			strip = 3;
		if ((angle >= 210) && (angle < 250))
			strip = 6;
		if ((angle >= 250) && (angle < 290))
			strip = 2;
		if ((angle >= 290) && (angle < 331))
			strip = 8;
	}

	if (strip > frameCount)
		strip = frameCount;

	_sceneObject->setStrip(strip);
}

/*--------------------------------------------------------------------------*/

SceneObject::SceneObject(): SceneHotspot() {
	_endAction = NULL;
	_mover = NULL;
	_objectWrapper = NULL;
	_flags = 0;
	_walkStartFrame = 0;
	_animateMode = ANIM_MODE_NONE;
	_updateStartFrame = 0;
	_moveDiff.x = 5;
	_moveDiff.y = 3;
	_numFrames = 10;
	_numFrames = 10;
	_field7A = 10;
	_regionBitList = 0;
	_sceneRegionId = 0;
	_percent = 100;
	_flags |= OBJFLAG_PANES;

	_frameChange = 0;
}

SceneObject::~SceneObject() {
	delete _mover;
	delete _objectWrapper;
}

int SceneObject::getNewFrame() {
	int frameNum = _frame + _frameChange;

	if (_frameChange > 0) {
		if (frameNum > getFrameCount()) {
			frameNum = 1;
			if (_animateMode == ANIM_MODE_1)
				++frameNum;
		}
	} else if (frameNum < 1) {
		frameNum = getFrameCount();
	}

	return frameNum;
}

int SceneObject::getFrameCount() {
	_visageImages.setVisage(_visage, _strip);
	return _visageImages.getFrameCount();
}

void SceneObject::animEnded() {
	_animateMode = ANIM_MODE_NONE;
	if (_endAction)
		_endAction->signal();
}

int SceneObject::changeFrame() {
	int frameNum = _frame;
	uint32 mouseCtr = _globals->_events.getFrameNumber();

	if ((_updateStartFrame <= mouseCtr) || (_animateMode == ANIM_MODE_1)) {
		if (_numFrames > 0) {
			int v = 60 / _numFrames;
			_updateStartFrame = mouseCtr + v;

			frameNum = getNewFrame();
		}
	}

	return frameNum;
}

void SceneObject::setPosition(const Common::Point &p, int yDiff) {
	_position = p;
	_yDiff = yDiff;
	_flags |= OBJFLAG_PANES;
}

void SceneObject::setZoom(int percent) {
	assert(_percent >= 0);
	if (percent != _percent) {
		_percent = percent;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::changeZoom(int percent) {
	if (percent == -1)
		_flags &= ~OBJFLAG_ZOOMED;
	else {
		_flags |= OBJFLAG_ZOOMED;
		setZoom(percent);
	}
}

void SceneObject::setStrip(int stripNum) {
	if (stripNum != _strip) {
		_strip = stripNum;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::setStrip2(int stripNum) {
	if (stripNum == -1)
		_flags &= ~OBJFLAG_8;
	else {
		_flags |= OBJFLAG_8;
		setStrip(stripNum);
	}
}

void SceneObject::setFrame(int frameNum) {
	if (frameNum != _frame) {
		_frame = frameNum;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::setFrame2(int frameNum) {
	if (frameNum != -1) {
		_flags |= OBJFLAG_NO_UPDATES;
		setFrame(frameNum);
	} else {
		_flags &= ~OBJFLAG_NO_UPDATES;
	}
}

void SceneObject::setPriority(int priority) {
	if (priority != _priority) {
		_priority = priority;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::setPriority2(int priority) {
	if (priority == -1) {
		_flags &= ~1;
	} else {
		_flags |= 1;
		setPriority(priority);
	}
}

void SceneObject::setVisage(int visage) {
	if (visage != _visage) {
		_visage = visage;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::setObjectWrapper(SceneObjectWrapper *objWrapper) {
	if (_objectWrapper)
		_objectWrapper->remove();
	_objectWrapper = objWrapper;
	if (objWrapper)
		objWrapper->setSceneObject(this);
}

void SceneObject::addMover(ObjectMover *mover, ...) {
	if (_mover) 
		_mover->remove();
	_mover = mover;

	if (mover) {
		// Set up the assigned mover
		_walkStartFrame = _globals->_events.getFrameNumber();
		if (_field7A != 0)
			_walkStartFrame = 60 / _field7A;

		// Signal the mover that movement is beginning
		va_list va;
		va_start(va, mover);
		mover->startMove(this, va);
		va_end(va);
	}
}

void SceneObject::getHorizBounds() {
	Rect tempRect;

	GfxSurface frame = getFrame();
	tempRect.resize(frame, _position.x, _position.y - _yDiff, _percent);

	_xs = tempRect.left;
	_xe = tempRect.right;
}

int SceneObject::checkRegion(const Common::Point &pt) {
	Rect tempRect;
	int regionIndex = 0;

	// Temporarily change the position
	Common::Point savedPos = _position;
	_position = pt;
	
	int regIndex = _globals->_sceneRegions.indexOf(pt);
	if (_regionBitList & (1 << regIndex))
		regionIndex = regIndex;

	// Restore position
	_position = savedPos;

	// Get the object's frame bounds
	GfxSurface frame = getFrame();
	tempRect.resize(frame, _position.x, _position.y - _yDiff, _percent);

	int yPos, newY;
	if ((_position.y - _yDiff) <= (pt.y - _yDiff)) {
		yPos = _position.y - _yDiff;
		newY = pt.y;
	} else {
		yPos = pt.y - _yDiff;
		newY = _position.y;
	}
	newY -= _yDiff;

	List<SceneObject *>::iterator i;
	for (i = _globals->_sceneObjects->begin(); (regionIndex == 0) && (i != _globals->_sceneObjects->end()); ++i) {
		if ((*i) && ((*i)->_flags & OBJFLAG_1000)) {
			int objYDiff = (*i)->_position.y - _yDiff;
			if ((objYDiff >= yPos) && (objYDiff <= newY) &&
				((*i)->_xs < tempRect.right) && ((*i)->_xe > tempRect.left)) {
				// Found index
				regionIndex = -1; //****DEBUG*** = *i;
				break;
			}
		}
	}

	return regionIndex;
}

void SceneObject::animate(AnimateMode animMode, ...) {
	_animateMode = animMode;
	_updateStartFrame = _globals->_events.getFrameNumber();
	if (_numFrames)
		_updateStartFrame += 60 / _numFrames;

	va_list va;
	va_start(va, animMode);

	switch (_animateMode) {
	case ANIM_MODE_NONE:
		_endAction = NULL;
		break;

	case ANIM_MODE_1:
		_frameChange = 1;
		_field2E = _position;
		_endAction = 0;
		break;

	case ANIM_MODE_2:
		_frameChange = 1;
		_endAction = NULL;
		break;

	case ANIM_MODE_3:
		_frameChange = -1;
		_endAction = NULL;
		break;

	case ANIM_MODE_4:
		_endFrame = va_arg(va, int);
		_frameChange = va_arg(va, int);
		_endAction = va_arg(va, Action *);
		if (_endFrame == _frame)
			setFrame(getNewFrame());
		break;

	case ANIM_MODE_5:
		_frameChange = 1;
		_endFrame = getFrameCount();
		_endAction = va_arg(va, Action *);
		if (_endFrame == _frame)
			setFrame(getNewFrame());
		break;

	case ANIM_MODE_6:
		_frameChange = -1;
		_endAction = va_arg(va, Action *);
		_endFrame = 1;
		if (_frame == _endFrame)
			setFrame(getNewFrame());
		break;

	case ANIM_MODE_7:
		_endFrame = va_arg(va, int);
		_endAction = va_arg(va, Action *);
		_frameChange = 1;
		break;

	case ANIM_MODE_8:
		_field68 = va_arg(va, int);
		_endAction = va_arg(va, Action *);
		_frameChange = 1;
		_endFrame = getFrameCount();
		if (_frame == _endFrame)
			setFrame(getNewFrame());
		break;		
	}
}

SceneObject *SceneObject::clone() const {
	SceneObject *obj = new SceneObject(*this);
	return obj;
}

void SceneObject::checkAngle(const SceneObject *obj) {
	_angle = GfxManager::getAngle(_position, obj->_position);

	if (_objectWrapper)
		_objectWrapper->dispatch();
}

void SceneObject::flag100() {
	_flags |= OBJFLAG_100;
	if (_flags & OBJFLAG_200)
		_flags |= OBJFLAG_PANES;
}

void SceneObject::unflag100() {
	if (_flags & OBJFLAG_100) {
		_flags &= ~OBJFLAG_100;
		_flags |= OBJFLAG_PANES;
	}
}

int SceneObject::getSpliceArea(const SceneObject *obj) {
	int xd = ABS(_position.x - obj->_position.x);
	int yd = ABS(_position.y - obj->_position.y);
	
	return (xd * xd + yd) / 2;
}

void SceneObject::synchronise(Serialiser &s) {
	SceneHotspot::synchronise(s);

	s.syncAsUint32LE(_updateStartFrame);
	s.syncAsUint32LE(_walkStartFrame);
	s.syncAsSint16LE(_field2E.x); s.syncAsSint16LE(_field2E.y);
	s.syncAsSint16LE(_percent);
	s.syncAsSint16LE(_priority);
	s.syncAsSint16LE(_angle);
	s.syncAsUint32LE(_flags);
	s.syncAsSint16LE(_xs);
	s.syncAsSint16LE(_xe);
	_paneRects[0].synchronise(s);
	_paneRects[1].synchronise(s);
	s.syncAsSint32LE(_visage);
	SYNC_POINTER(_objectWrapper);
	s.syncAsSint32LE(_strip);
	SYNC_ENUM(_animateMode, AnimateMode);
	s.syncAsSint32LE(_frame);
	s.syncAsSint32LE(_endFrame);
	s.syncAsSint32LE(_field68);
	s.syncAsSint32LE(_frameChange);
	s.syncAsSint32LE(_numFrames);
	s.syncAsSint32LE(_field6E);
	SYNC_POINTER(_mover);
	s.syncAsSint16LE(_moveDiff.x); s.syncAsSint16LE(_moveDiff.y);
	s.syncAsSint32LE(_field7A);
	SYNC_POINTER(_endAction);
	s.syncAsUint32LE(_regionBitList);	
}

void SceneObject::postInit(SceneObjectList *OwnerList) {
	if (!OwnerList)
		OwnerList = _globals->_sceneObjects;

	if (!OwnerList->contains(this)) {
		_percent = 100;
		_priority = 255;
		_flags = 4;
		_visage = 0;
		_strip = 1;
		_frame = 1;
		_objectWrapper = NULL;
		_animateMode = ANIM_MODE_NONE;
		_endAction = 0;
		_mover = NULL;
		_yDiff = 0;
		_moveDiff.x = 5;
		_moveDiff.y = 3;
		_field7A = 10;
		_field6E = 64;
		_numFrames = 10;
		_regionBitList = 0;

		OwnerList->push_back(this);
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::remove() {
	SceneItem::remove();
	if (_globals->_sceneObjects->contains(this))
		// For objects in the object list, flag the object for removal in the next drawing, so that 
		// the drawing code has a chance to restore the area previously covered by the object
		_flags |= OBJFLAG_PANES | OBJFLAG_REMOVE | OBJFLAG_100;
	else
		// Not in the list, so immediately remove the object
		removeObject();
}

void SceneObject::dispatch() {
	uint32 currTime = _globals->_events.getFrameNumber();
	if (_action)
		_action->dispatch();

	if (_mover && (_walkStartFrame <= currTime)) {
		if (_field7A) {
			int frameInc = 60 / _field7A;
			_walkStartFrame = currTime + frameInc;
		}
		_mover->dispatch();
	}

	if (!(_flags & OBJFLAG_NO_UPDATES)) {
		switch (_animateMode) {
		case ANIM_MODE_1:
			if (isNoMover())
				setFrame(1);
			else if ((_field2E.x != _position.x) || (_field2E.y != _position.y)) {
				setFrame(changeFrame());
				_field2E = _position;

			}
			break;

		case ANIM_MODE_2:
		case ANIM_MODE_3:
			setFrame(changeFrame());

			break;
		case ANIM_MODE_4:
		case ANIM_MODE_5:
		case ANIM_MODE_6:
			if (_frame == _endFrame)
				animEnded();
			else
				setFrame(changeFrame());
			break;

		case ANIM_MODE_7:
			if (changeFrame() != _frame) {
				// Pick a new random frame
				int frameNum = 0;
				do {
					int count = getFrameCount();
					frameNum = _globals->_randomSource.getRandomNumber(count - 1);
				} while (frameNum == _frame);

				setFrame(frameNum);
				if (_endFrame) {
					if (--_endFrame == 0)
						animEnded();
				}
			}
			break;

		case ANIM_MODE_8:
			if (_frame == _endFrame) {
				if (_frameChange != -1) {
					_frameChange = -1;
					_endFrame = 1;

					setFrame(changeFrame());
				} else if (!_field68 || (--_field68 > 0)) {
					_frameChange = 1;
					_endFrame = getFrameCount();

					setFrame(changeFrame());
				} else {
					animEnded();
				}
			} else {
				setFrame(changeFrame());
			}
			
			break;

		default:
			break;
		}
	}

	// Handle updating the zoom and/or priority
	if (!(_flags & OBJFLAG_ZOOMED)) {
		int yp = MIN((int)_position.y, 255);
		setZoom(_globals->_sceneManager._scene->_zoomPercents[yp]);
	}
	if (!(_flags & OBJFLAG_FIXED_PRIORITY)) {
		setPriority(_position.y);
	}
}

void SceneObject::calcAngle(const Common::Point &pt) {
	int newAngle = GfxManager::getAngle(_position, pt);
	if (newAngle != -1)
		_angle = newAngle;
}

void SceneObject::removeObject() {
	if (_globals->_sceneItems.contains(this))
		_globals->_sceneItems.remove(this);

	if (_globals->_sceneObjects->contains(this))
		_globals->_sceneObjects->remove(this);

	if (_visage) {
		_vm->_memoryManager.deallocate(_visage);
		_visage = 0;
	}

	if (_objectWrapper) {
		_objectWrapper->remove();
		_objectWrapper = NULL;
	}
	if (_mover) {
		_mover->remove();
		_mover = NULL;
	}
	if (_flags & 0x800)
		destroy();
}

GfxSurface SceneObject::getFrame() {
	_visageImages.setVisage(_visage, _strip);
	return _visageImages.getFrame(_frame);
}

void SceneObject::reposition() {
	GfxSurface frame = getFrame();
	_bounds.resize(frame, _position.x, _position.y - _yDiff, _percent);
	_xs = _bounds.left;
	_xe = _bounds.right;
}

/**
 * Draws an object into the scene
 */
void SceneObject::draw() {
	Rect destRect = _bounds;
	destRect.translate(-_globals->_sceneManager._scene->_sceneBounds.left, 
		-_globals->_sceneManager._scene->_sceneBounds.top);
	Region *priorityRegion = _globals->_sceneManager._scene->_priorities.find(_priority);
	GfxSurface frame = getFrame();
	_globals->gfxManager().copyFrom(frame, destRect, priorityRegion); 
}

/**
 * Refreshes the background around the area of a scene object prior to it's being redrawn, 
 * in case it is moving
 */
void SceneObject::updateScreen() {
	Rect srcRect = _paneRects[CURRENT_PANENUM];
	const Rect &sceneBounds = _globals->_sceneManager._scene->_sceneBounds;
	srcRect.left = (srcRect.left / 4) * 4;
	srcRect.right = ((srcRect.right + 3) / 4) * 4;
	srcRect.clip(_globals->_sceneManager._scene->_sceneBounds);

	if (srcRect.isValidRect()) {
		Rect destRect  = srcRect;
		destRect.translate(-_globals->_sceneOffset.x, -_globals->_sceneOffset.y);
		destRect.translate(-sceneBounds.left, -sceneBounds.top);

		_globals->_screenSurface.copyFrom(_globals->_sceneManager._scene->_backSurface, srcRect, destRect);
	}
}

/*--------------------------------------------------------------------------*/

void SceneObjectList::draw() {
	Common::Array<SceneObject *> objList;
	int paneNum = 0;
	int xAmount = 0, yAmount = 0;

	if (_objList.size() == 0) {
		// Alternate draw mode

		if (_globals->_paneRefreshFlag[paneNum] == 1) {
			// Load the background
			_globals->_sceneManager._scene->refreshBackground(0, 0);

			Rect tempRect = _globals->_sceneManager._scene->_sceneBounds;
			tempRect.translate(-_globals->_sceneOffset.x, -_globals->_sceneOffset.y);
			ScenePalette::changeBackground(tempRect, _globals->_sceneManager._fadeMode);
		} else {
			_globals->_paneRegions[CURRENT_PANENUM].draw();
		}

		_globals->_paneRegions[CURRENT_PANENUM].setRect(0, 0, 0, 0);
		_globals->_sceneManager.fadeInIfNecessary();

	} else {
		// If there is a scroll follower, check whether it has moved off-screen
		if (_globals->_scrollFollower) {
			const Common::Point &objPos = _globals->_scrollFollower->_position;
			const Rect &scrollerRect = _globals->_sceneManager._scrollerRect;
			int loadCount = 0;

			if (objPos.x >= scrollerRect.right) {
				xAmount = 8;
				loadCount = 20;
			}
			if (objPos.x < scrollerRect.left) {
				xAmount = -8;
				loadCount = 20;
			}
			if (objPos.y >= scrollerRect.bottom) {
				yAmount = 2;
				loadCount = 25;
			}
			if (objPos.y < scrollerRect.top) {
				yAmount = -2;
				loadCount = 25;
			}

			if (loadCount > 0)
				_globals->_sceneManager.setBgOffset(Common::Point(xAmount, yAmount), loadCount);
		}

		if (_globals->_sceneManager._sceneLoadCount > 0) {
			--_globals->_sceneManager._sceneLoadCount;
			_globals->_sceneManager._scene->loadBackground(_globals->_sceneManager._sceneBgOffset.x,
				_globals->_sceneManager._sceneBgOffset.y);
		}

		// Set up the flag mask
		uint32 flagMask = (paneNum == 0) ? OBJFLAG_PANE_0 : OBJFLAG_PANE_1;

		// Initial loop to set up object list and update object position, priority, and flags
		for (List<SceneObject *>::iterator i = _globals->_sceneObjects->begin();
				i != _globals->_sceneObjects->end(); ++i) {
			SceneObject *obj = *i;
			objList.push_back(obj);

			if (!(obj->_flags & OBJFLAG_100))
				obj->_flags &= ~OBJFLAG_200;

			// Reposition the bounds of the object to match the desired position
			obj->reposition();

			// Handle updating object priority
			if (!(obj->_flags & OBJFLAG_FIXED_PRIORITY)) {
				obj->_priority = MIN((int)obj->_position.y - 1, 
					(int)_globals->_sceneManager._scene->_backgroundBounds.bottom);
			}

			if ((_globals->_paneRefreshFlag[paneNum] != 0) || !_globals->_paneRegions[paneNum].empty()) {
				obj->_flags |= flagMask;
			}
		}

		// Check for any intersections, and then sort the object list by priority
		checkIntersection(objList, objList.size(), CURRENT_PANENUM);
		sortList(objList);

		if (_globals->_paneRefreshFlag[paneNum] == 1) {
			// Load the background
			_globals->_sceneManager._scene->refreshBackground(0, 0);
		}

		_globals->_sceneManager._scene->_sceneBounds.left &= ~3;
		_globals->_sceneManager._scene->_sceneBounds.right &= ~3;
		_globals->_sceneOffset.x &= ~3;

		if (_globals->_paneRefreshFlag[paneNum] != 0) {
			// Change the background
			Rect tempRect = _globals->_sceneManager._scene->_sceneBounds;
			tempRect.translate(-_globals->_sceneOffset.x, -_globals->_sceneOffset.y);
			ScenePalette::changeBackground(tempRect, _globals->_sceneManager._fadeMode);
		} else {
			for (uint objIndex = 0; objIndex < objList.size(); ++objIndex) {
				SceneObject *obj = objList[objIndex];
				
				if ((obj->_flags & flagMask) && obj->_paneRects[paneNum].isValidRect())
					obj->updateScreen();
			}

			_globals->_paneRegions[paneNum].draw();
		}

		_globals->_paneRegions[paneNum].setRect(0, 0, 0, 0);
redraw:
		// Main draw loop
		for (uint objIndex = 0; objIndex < objList.size(); ++objIndex) {
			SceneObject *obj = objList[objIndex];

			if ((obj->_flags & flagMask) && !(obj->_flags & OBJFLAG_100)) {
				obj->_paneRects[paneNum] = obj->_bounds;
				obj->draw();
			}
		}

		// Update the palette
		_globals->_sceneManager.fadeInIfNecessary();
		_globals->_sceneManager._loadMode = 0;
		_globals->_paneRefreshFlag[paneNum] = 0;

		// Loop through the object list, removing any objects and refreshing the screen as necessary
		for (uint objIndex = 0; objIndex < objList.size(); ++objIndex) {
			SceneObject *obj = objList[objIndex];

			if (obj->_flags & OBJFLAG_100)
				obj->_flags |= OBJFLAG_200;
			obj->_flags &= ~flagMask;
			if (obj->_flags & OBJFLAG_REMOVE) {
				obj->_flags |= OBJFLAG_PANES;
				
				checkIntersection(objList, objIndex, CURRENT_PANENUM);

				obj->updateScreen();
				obj->removeObject();

				// FIXME: Currently, removing objects causes screen flickers when the removed object intersects
				// another drawn object, since the background is briefly redrawn over the object. For now, I'm
				// using a forced jump back to redraw objects. In the long term, I should figure out how the
				// original game does this properly
				objList.remove_at(objIndex);
				goto redraw;
			}
		}
	}
}

void SceneObjectList::checkIntersection(Common::Array<SceneObject *> &ObjList, uint ObjIndex, int PaneNum) {
	uint32 flagMask = (PaneNum == 0) ? OBJFLAG_PANE_0 : OBJFLAG_PANE_1;
	SceneObject *obj = (ObjIndex == ObjList.size()) ? NULL : ObjList[ObjIndex];
	Rect rect1;

	for (uint idx = 0; idx < ObjList.size(); ++idx) {
		SceneObject *currObj = ObjList[idx];

		if (ObjIndex == ObjList.size()) {
			if (currObj->_flags & flagMask)
				checkIntersection(ObjList, idx, PaneNum);
		} else if (idx != ObjIndex) {
			Rect &paneRect = obj->_paneRects[PaneNum];
			Rect objBounds = currObj->_bounds;
			if (paneRect.isValidRect())
				objBounds.extend(paneRect);

			Rect objBounds2 = currObj->_bounds;
			if (paneRect.isValidRect())
				objBounds2.extend(paneRect);

			objBounds.left &= ~3;
			objBounds.right += 3;
			objBounds.right &= ~3;
			objBounds2.left &= ~3;
			objBounds2.right += 3;
			objBounds2.right &= ~3;

			if (objBounds.intersects(objBounds2) && !(currObj->_flags & flagMask)) {
				currObj->_flags |= flagMask;
				checkIntersection(ObjList, idx, PaneNum);
			}
		}
	}
}

struct SceneObjectLess {
	bool operator()(const SceneObject *x, const SceneObject *y) const {
		if (y->_priority > x->_priority)
			return true;
		else if ((y->_priority == x->_priority) && (y->_position.y > x->_position.y))
			return true;
		else if ((y->_priority == x->_priority) && (y->_position.y == x->_position.y) &&
				 (y->_yDiff > x->_yDiff))
			return true;

		return false;
	}
};

void SceneObjectList::sortList(Common::Array<SceneObject *> &ObjList) {
	Common::sort(ObjList.begin(), ObjList.end(), SceneObjectLess());
}

void SceneObjectList::activate() {
	SceneObjectList *objectList = _globals->_sceneObjects;
	_globals->_sceneObjects = this;
	_globals->_sceneObjects_queue.push_front(this);

	// Flag all the objects as modified
	List<SceneObject *>::iterator i;
	for (i = begin(); i != end(); ++i) {
		(*i)->_flags |= OBJFLAG_PANES;
	}

	// Replicate all existing objects on the old object list
	for (i = objectList->begin(); i != objectList->end(); ++i) {
		SceneObject *sceneObj = (*i)->clone();
		sceneObj->_flags |= OBJFLAG_100 | OBJFLAG_REMOVE | OBJFLAG_800;
		push_front(sceneObj);
	}
}

void SceneObjectList::deactivate() {
	if (_globals->_sceneObjects_queue.size() <= 1)
		return;

	SceneObjectList *objectList = *_globals->_sceneObjects_queue.begin();
	_globals->_sceneObjects_queue.pop_front();
	_globals->_sceneObjects = *_globals->_sceneObjects_queue.begin();

	List<SceneObject *>::iterator i;
	for (i = objectList->begin(); i != objectList->end(); ++i) {
		if (!((*i)->_flags & OBJFLAG_800)) {
			SceneObject *sceneObj = (*i)->clone();
			sceneObj->_flags |= OBJFLAG_100 | OBJFLAG_REMOVE | OBJFLAG_800;
			_globals->_sceneObjects->push_front(sceneObj);
		}
	}
}

void SceneObjectList::synchronise(Serialiser &s) {
	_objList.synchronise(s);
}

/*--------------------------------------------------------------------------*/

SceneText::SceneText(): SceneObject() {
	_fontNumber = 2;
	_width = 160;
	_textMode = ALIGN_LEFT;
	_colour2 = 0;
	_colour3 = 0;
}

SceneText::~SceneText() {
}

void SceneText::setup(const Common::String &msg) {
	GfxManager gfxMan(_textSurface);
	gfxMan.activate();
	Rect textRect;
	
	gfxMan._font.setFontNumber(_fontNumber);
	gfxMan._font._colours.foreground = _colour1;
	gfxMan._font._colours2.background = _colour2;
	gfxMan._font._colours2.foreground = _colour3;

	gfxMan.getStringBounds(msg.c_str(), textRect, _width);
	_bounds = textRect;

	// Set up a new blank surface to hold the text
	_textSurface.create(textRect.width(), textRect.height());
	_textSurface._transColour = 0xff;
	_textSurface.fillRect(textRect, _textSurface._transColour);

	// Write the text to the surface
	gfxMan._font.writeLines(msg.c_str(), textRect, _textMode);	

	// Do post-init, which adds this SceneText object to the scene
	postInit();
	gfxMan.deactivate();
}

void SceneText::synchronise(Serialiser &s) {
	SceneObject::synchronise(s);

	s.syncAsSint16LE(_fontNumber);
	s.syncAsSint16LE(_width);
	s.syncAsSint16LE(_colour1);
	s.syncAsSint16LE(_colour2);
	s.syncAsSint16LE(_colour3);
	SYNC_ENUM(_textMode, TextAlign);
}

/*--------------------------------------------------------------------------*/

Visage::Visage() {
	_resNum = 0;
	_rlbNum = 0;
	_data = NULL;
}

void Visage::setVisage(int resNum, int rlbNum) {
	if ((_resNum != resNum) || (_rlbNum != rlbNum)) {
		_resNum = resNum;
		_rlbNum = rlbNum;
		DEALLOCATE(_data);
		_data = _vm->_dataManager->getResource(RES_VISAGE, resNum, rlbNum);
		assert(_data);
	}
}

Visage::~Visage() {
	DEALLOCATE(_data);
}

GfxSurface Visage::getFrame(int frameNum) {
	int numFrames = READ_LE_UINT16(_data);
	if (frameNum > numFrames)
		frameNum = numFrames;
	if (frameNum > 0)
		--frameNum;

	int offset = READ_UINT32(_data + 2 + frameNum * 4);
	byte *frameData = _data + offset;

	return surfaceFromRes(frameData);
}

int Visage::getFrameCount() const {
	return READ_LE_UINT16(_data);
}

/*--------------------------------------------------------------------------*/

void Player::postInit(SceneObjectList *OwnerList) {
	SceneObject::postInit();

	_canWalk = true;
	_uiEnabled = true;
	_percent = 100;
	_field8C = 10;
	_moveDiff.x = 4;
	_moveDiff.y = 2;
}

void Player::disableControl() {
	_canWalk = false;
	_uiEnabled = false;
	_globals->_events.setCursor(CURSOR_NONE);
}

void Player::enableControl() {
	_canWalk = true;
	_uiEnabled = true;
	_globals->_events.setCursor(CURSOR_WALK);

	switch (_globals->_events.getCursor()) {
	case CURSOR_WALK:
	case CURSOR_LOOK:
	case CURSOR_USE:
	case CURSOR_TALK:
		_globals->_events.setCursor(_globals->_events.getCursor());
		break;
	default:
		_globals->_events.setCursor(CURSOR_WALK);
		break;
	}
}

void Player::process(Event &event) {
	if (!event.handled && (event.eventType == EVENT_BUTTON_DOWN) && 
			(_globals->_events.getCursor() == CURSOR_WALK) && _globals->_player._canWalk &&
			(_position != event.mousePos) && _globals->_sceneObjects->contains(this)) {

		PlayerMover *newMover = new PlayerMover();
		Common::Point destPos(event.mousePos.x - _globals->_sceneManager._scene->_sceneBounds.left,
			event.mousePos.y - _globals->_sceneManager._scene->_sceneBounds.top);

		addMover(newMover, &destPos, NULL);
		event.handled = true;
	}
}

void Player::synchronise(Serialiser &s) {
	SceneObject::synchronise(s);

	s.syncAsByte(_canWalk);
	s.syncAsByte(_uiEnabled);
	s.syncAsSint16LE(_field8C);
}

/*--------------------------------------------------------------------------*/

Region::Region(int resNum, int rlbNum, ResourceType ctlType) {
	_regionId = rlbNum;

	byte *regionData = _vm->_dataManager->getResource(ctlType, resNum, rlbNum);
	assert(regionData);

	// Set the region bounds
	_bounds.top = READ_LE_UINT16(regionData + 6);
	_bounds.left = READ_LE_UINT16(regionData + 8);
	_bounds.bottom = READ_LE_UINT16(regionData + 10);
	_bounds.right = READ_LE_UINT16(regionData + 12);

	// Special handling for small size regions
	_regionSize = READ_LE_UINT16(regionData);
	if (_regionSize == 14)
		// No line slices
		return;

	// Set up the line slices
	for (int y = 0; y < (_regionSize == 22 ? 1 : _bounds.height()); ++y) {
		int slicesCount = READ_LE_UINT16(regionData + 16 + y * 4);
		int slicesOffset = READ_LE_UINT16(regionData + 14 + y * 4);
		assert(slicesCount < 100);
		LineSliceSet sliceSet;
		sliceSet.load(slicesCount, regionData + 14 + slicesOffset);

		_ySlices.push_back(sliceSet);
	}

	DEALLOCATE(regionData);
}

/**
 * Returns true if the given region contains the specified point
 * @param pt	Specified position
 */
bool Region::contains(const Common::Point &pt) {
	// First check if the point falls inside the overall bounding rectangle
	if (!_bounds.contains(pt) || _ySlices.empty())
		return false;

	// Get the correct Y line to use
	const LineSliceSet &line = getLineSlices(pt.y);

	// Loop through the horizontal slice list to see if the point falls in one
	for (uint idx = 0; idx < line.items.size(); ++idx) {
		if ((pt.x >= line.items[idx].xs) && (pt.x < line.items[idx].xe))
			return true;
	}

	return false;
}

/**
 * Returns true if the given region is empty
 */
bool Region::empty() const {
	return !_bounds.isValidRect() && (_regionSize == 14);
}

void Region::clear() {
	_bounds.set(0, 0, 0, 0);
	_regionId = 0;
	_regionSize = 0;
}

void Region::setRect(const Rect &r) {
	setRect(r.left, r.top, r.right, r.bottom);
}

void Region::setRect(int xs, int ys, int xe, int ye) {
	bool validRect = (ys < ye) && (xs < xe);
	_ySlices.clear();

	if (!validRect) {
		_regionSize = 14;
		_bounds.set(0, 0, 0, 0);
	} else {
		_regionSize = 22;
		_bounds.set(xs, ys, xe, ye);
		
		LineSliceSet sliceSet;
		sliceSet.load2(1, xs, xe);

		_ySlices.push_back(sliceSet);
	}
}

const LineSliceSet &Region::getLineSlices(int yp) {
	return _ySlices[(_regionSize == 22) ? 0 : yp - _bounds.top];	
}

LineSliceSet Region::sectPoints(int yp, const LineSliceSet &sliceSet) {
	if ((yp < _bounds.top) || (yp >= _bounds.bottom))
		return LineSliceSet();

	const LineSliceSet &ySet = getLineSlices(yp);
	return mergeSlices(sliceSet, ySet);
}

LineSliceSet Region::mergeSlices(const LineSliceSet &set1, const LineSliceSet &set2) {
	LineSliceSet result;

	uint set1Index = 0, set2Index = 0;

	while ((set1Index < set1.items.size()) && (set2Index < set2.items.size())) {
		if (set1.items[set1Index].xe <= set2.items[set2Index].xs) {
			++set1Index;
		} else if (set2.items[set2Index].xe <= set1.items[set1Index].xs) {
			++set2Index;
		} else {
			bool set1Flag = set1.items[set1Index].xs >= set2.items[set2Index].xs;
			const LineSlice &slice = set1Flag ? set1.items[set1Index] : set2.items[set2Index];
						
			result.add(slice.xs, MIN(set1.items[set1Index].xe, set2.items[set2Index].xe));
			if (set1Flag)
				++set1Index;
			else
				++set2Index;
		}
	}

	return result;
}

/**
 * Copies the background covered by the given region to the screen surface
 */
void Region::draw() {
	Rect &sceneBounds = _globals->_sceneManager._scene->_sceneBounds;

	for (int yp = sceneBounds.top; yp < sceneBounds.bottom; ++yp) {
		// Generate a line slice set
		LineSliceSet tempSet;
		tempSet.add(sceneBounds.left, sceneBounds.right);
		LineSliceSet newSet = sectPoints(yp, tempSet);
		
		// Loop through the calculated slices
		for (uint idx = 0; idx < newSet.items.size(); ++idx) {
			Rect rect1(newSet.items[idx].xs, yp, newSet.items[idx].xe, yp + 1);
			rect1.left &= ~3;
			rect1.right = (rect1.right + 3) & ~3;
			
			Rect rect2 = rect1;
			rect1.translate(-_globals->_sceneOffset.x, -_globals->_sceneOffset.y);
			rect2.translate(-sceneBounds.left, -sceneBounds.top);

			_globals->gfxManager().getSurface().copyFrom(_globals->_sceneManager._scene->_backSurface,
				rect1, rect2);
		}
	}
}

void Region::uniteLine(int yp, LineSliceSet &sliceSet) {
	// TODO: More properly implement like the original
	
	// First expand the bounds as necessary to fit in the row
	if (_ySlices.empty()) {
		_bounds = Rect(sliceSet.items[0].xs, yp, sliceSet.items[sliceSet.items.size() - 1].xe, yp + 1);
		_ySlices.push_back(LineSliceSet());
	}
	while (yp < _bounds.top) {
		_ySlices.insert_at(0, LineSliceSet());
		--_bounds.top;
	}
	while (yp >= _bounds.bottom) {
		_ySlices.push_back(LineSliceSet());
		++_bounds.bottom;
	}

	// Merge the existing line set into the line
	LineSliceSet &destSet = _ySlices[yp - _bounds.top];
	for (uint srcIndex = 0; srcIndex < sliceSet.items.size(); ++srcIndex) {
		LineSlice &srcSlice = sliceSet.items[srcIndex];

		// Check if overlaps existing slices
		uint destIndex = 0;
		while (destIndex < destSet.items.size()) {
			LineSlice &destSlice = destSet.items[destIndex];
			if (((srcSlice.xs >= destSlice.xs) && (srcSlice.xs <= destSlice.xe)) ||
				((srcSlice.xe >= destSlice.xs) && (srcSlice.xe <= destSlice.xe)) ||
				((srcSlice.xs < destSlice.xs) && (srcSlice.xe > destSlice.xe))) {
				// Intersecting, so merge them
				destSlice.xs = MIN(srcSlice.xs, destSlice.xs);
				destSlice.xe = MAX(srcSlice.xe, destSlice.xe);
				break;
			}
			++destIndex;
		}
		if (destIndex == destSet.items.size()) {
			// No intersecting region found, so add it to the list
			destSet.items.push_back(srcSlice);
		}
	}

	// Check whether to expand the left/bounds bounds
	if (destSet.items[0].xs < _bounds.left)
		_bounds.left = destSet.items[0].xs;
	if (destSet.items[destSet.items.size() - 1].xe > _bounds.right)
		_bounds.right = destSet.items[destSet.items.size() - 1].xe;
}

void Region::uniteRect(const Rect &rect) {
	for (int yp = rect.top; yp < rect.bottom; ++yp) {
		LineSliceSet sliceSet;
		sliceSet.add(rect.left, rect.right);
		uniteLine(yp, sliceSet);
	}
}

/*--------------------------------------------------------------------------*/

void SceneRegions::load(int sceneNum) {
	clear();

	byte *regionData = _vm->_dataManager->getResource(RES_CONTROL, sceneNum, 9999, true);
	
	if (regionData) {
		int regionCount = READ_LE_UINT16(regionData);
		for (int regionCtr = 0; regionCtr < regionCount; ++regionCtr) {
			int rlbNum = READ_LE_UINT16(regionData + regionCtr * 6 + 2);

			push_back(Region(sceneNum, rlbNum));
		}

		DEALLOCATE(regionData);
	}
}

int SceneRegions::indexOf(const Common::Point &pt) {
	for (SceneRegions::iterator i = begin(); i != end(); ++i) {
		if ((*i).contains(pt))
			return (*i)._regionId;
	}

	return 0;
}

/*--------------------------------------------------------------------------*/

SoundHandler::SoundHandler() {
	_action = NULL;
	_field280 = -1;
	if (_globals)
		_globals->_sceneListeners.push_back(this);
}

SoundHandler::~SoundHandler() {
	if (_globals)
		_globals->_sceneListeners.remove(this);
}

void SoundHandler::dispatch() {
	EventHandler::dispatch();
	int v = _sound.proc12();

	if (v != -1) {
		_field280 = v;
		_sound.proc2(-1);

		if (_action)
			_action->signal();
	}

	if (_field280 != -1) {
		// FIXME: Hardcoded to only flag a sound ended if an action has been set
		if (_action) {
//		if (!_sound.proc3()) {
			_field280 = -1;
			if (_action) {
				_action->signal();
				_action = NULL;
			}
		}
	}
}

void SoundHandler::startSound(int soundNum, Action *action, int volume) {
	_action = action;
	_field280 = 0;
	setVolume(volume);
	_sound.startSound(soundNum);

	warning("TODO: SoundHandler::startSound");
}


/*--------------------------------------------------------------------------*/

void SceneItemList::addItems(SceneItem *first, ...) {
	va_list va;
	va_start(va, first);

	SceneItem *p = first;
	while (p) {
		push_back(p);
		p = va_arg(va, SceneItem *);
	}
}

/*--------------------------------------------------------------------------*/

RegionSupportRec WalkRegion::_processList[PROCESS_LIST_SIZE];

void RegionSupportRec::process() {
	if (_xDiff < _yDiff) {
		_halfDiff += _xDiff;
		if (_halfDiff > _yDiff) {
			_halfDiff -= _yDiff;
			_xp += _xDirection;
		}
	} else {
		do {
			_xp += _xDirection;
			_halfDiff += _yDiff;
		} while (_halfDiff <= _xDiff);
		_halfDiff -= _xDiff;
	}
	--_yDiff2;
}

/*--------------------------------------------------------------------------*/

void WalkRegion::loadRegion(byte *dataP, int size) {
	// First clear the region
	clear();

	// Decode the data for the region
	int dataCount, regionHeight;
	loadProcessList(dataP, size, dataCount, regionHeight);

	int processIndex = 0, idx2 = 0, count;
	for (int yp = _processList[0]._yp; yp < regionHeight; ++yp) {
		process3(yp, dataCount, processIndex, idx2);
		process4(yp, processIndex, idx2, count);

		loadRecords(yp, count, processIndex); 
	}
}

void WalkRegion::loadProcessList(byte *dataP, int dataSize, int &dataIndex, int &regionHeight) {
	dataIndex = 0;
	int x1 = READ_LE_UINT16(dataP + (dataSize - 1) * 4);
	int y1 = READ_LE_UINT16(dataP + (dataSize - 1) * 4 + 2);
	regionHeight = y1;

	for (int idx = 0; idx < dataSize; ++idx) {
		int xp = READ_LE_UINT16(dataP + idx * 4);
		int yp = READ_LE_UINT16(dataP + idx * 4 + 2);
		if (yp != y1) {
			/*
			 * Commented out: doesn't seem to be used
			int v;
			if (idx == (dataSize - 1))
				v = READ_LE_UINT16(dataP + 2);
			else 
				v = process1(idx, dataP, dataSize);
			warning("TODO: v not used? - %d", v);
			*/
			process2(dataIndex, x1, y1, xp, yp);
			++dataIndex;
		}

		// Keep regionHeight as the maximum of any y
		if (yp > regionHeight)
			regionHeight = yp;

		x1 = xp;
		y1 = yp;
	}
}

int WalkRegion::process1(int idx, byte *dataP, int dataSize) {
	int idx2 = idx + 1;
	if (idx2 == dataSize)
		idx2 = 0;

	while (READ_LE_UINT16(dataP + idx2 * 4 + 2) == READ_LE_UINT16(dataP + idx * 4 + 2)) {
		if (idx2 == (dataSize - 1))
			idx2 = 0;
		else
			++idx2;
	}

	return READ_LE_UINT16(dataP + idx2 * 4 + 2);
}

void WalkRegion::process2(int dataIndex, int x1, int y1, int x2, int y2) {
	int xDiff = ABS(x2 - x1);
	int yDiff = ABS(y2 - y1);
	int halfDiff = MAX(xDiff, yDiff) / 2;
	int yMax = MIN(y1, y2);
	
	while (dataIndex && (_processList[dataIndex - 1]._yp > yMax)) {
		_processList[dataIndex] = _processList[dataIndex - 1];
		--dataIndex;
	}
	_processList[dataIndex]._yp = yMax;

	_processList[dataIndex]._xp = (y1 >= y2) ? x2 : x1;
	_processList[dataIndex]._xDiff = xDiff;
	_processList[dataIndex]._yDiff = yDiff;
	_processList[dataIndex]._halfDiff = halfDiff;

	int xTemp = (y1 >= y2) ? x1 - x2 : x2 - x1;
	_processList[dataIndex]._xDirection = (xTemp == 0) ? 0 : ((xTemp < 0) ? -1 : 1);
	_processList[dataIndex]._yDiff2 = yDiff;
}

void WalkRegion::process3(int yp, int dataCount, int &idx1, int &idx2) {
	while ((idx2 < (dataCount - 1)) && (_processList[idx2 + 1]._yp <= yp))
		++idx2;
	while (!_processList[idx1]._yDiff2)
		++idx1;
}

void WalkRegion::process4(int yp, int idx1, int idx2, int &count) {
	count = 0;
	for (int idx = idx1; idx <= idx2; ++idx) {
		if (_processList[idx]._yDiff2 > 0)
			++count;
		process5(idx, idx1);
	}
}

void WalkRegion::process5(int idx1, int idx2) {
	while ((idx1 > idx2) && (_processList[idx1 - 1]._xp > _processList[idx1]._xp)) {
		SWAP(_processList[idx1], _processList[idx1 - 1]);
		--idx1;
	}
}

void WalkRegion::loadRecords(int yp, int size, int processIndex) {
	LineSliceSet sliceSet;
	int sliceCount =  size / 2;

	for (int idx = 0; idx < sliceCount; ++idx, ++processIndex) {
		while (!_processList[processIndex]._yDiff2)
			++processIndex;

		int sliceXs = _processList[processIndex]._xp;
		_processList[processIndex].process();

		do {
			++processIndex;
		} while (!_processList[processIndex]._yDiff2);

		int sliceXe = _processList[processIndex]._xp;
		_processList[processIndex].process();

		sliceSet.items.push_back(LineSlice(sliceXs, sliceXe));
	}

	uniteLine(yp, sliceSet);
}

/*--------------------------------------------------------------------------*/

void WRField18::load(byte *data) {
	_pt1.x = READ_LE_UINT16(data);
	_pt1.y = READ_LE_UINT16(data + 2);
	_pt2.x = READ_LE_UINT16(data + 4);
	_pt2.y = READ_LE_UINT16(data + 6);
	_v = READ_LE_UINT16(data + 8);
}

/*--------------------------------------------------------------------------*/

void WalkRegions::clear() {
	_regionList.clear();
	_field18.clear();
	_idxList.clear();
	_idxList2.clear();
}

void WalkRegions::load(int sceneNum) {
	clear();

	_resNum = sceneNum;
	byte *regionData = _vm->_dataManager->getResource(RES_WALKRGNS, sceneNum, 1, true);
	if (!regionData)
		// No data, so return
		return;

	byte *dataP;
	int dataSize;

	// Load the field 18 list
	dataP = _vm->_dataManager->getResource(RES_WALKRGNS, sceneNum, 2);
	dataSize = _vm->_memoryManager.getSize(dataP);
	assert(dataSize % 10 == 0);

	byte *p = dataP;
	for (int idx = 0; idx < (dataSize / 10); ++idx, p += 10) {
		WRField18 rec;
		rec.load(p);
		_field18.push_back(rec);
	}

	DEALLOCATE(dataP);
	
	// Load the idx list
	dataP = _vm->_dataManager->getResource(RES_WALKRGNS, sceneNum, 3);
	dataSize = _vm->_memoryManager.getSize(dataP);
	assert(dataSize % 2 == 0);

	p = dataP;
	for (int idx = 0; idx < (dataSize / 2); ++idx, p += 2)
		_idxList.push_back(READ_LE_UINT16(p));

	DEALLOCATE(dataP);

	// Load the secondary idx list
	dataP = _vm->_dataManager->getResource(RES_WALKRGNS, sceneNum, 4);
	dataSize = _vm->_memoryManager.getSize(dataP);
	assert(dataSize % 2 == 0);

	p = dataP;
	for (int idx = 0; idx < (dataSize / 2); ++idx, p += 2)
		_idxList2.push_back(READ_LE_UINT16(p));

	DEALLOCATE(dataP);

	// Handle the loading of the actual regions themselves
	dataP = _vm->_dataManager->getResource(RES_WALKRGNS, sceneNum, 5);

	byte *pWalkRegion = regionData + 16;
	byte *srcP = dataP;
	for (; (int16)READ_LE_UINT16(pWalkRegion) != -20000; pWalkRegion += 16) {
		WalkRegion wr;

		// Set the Walk region specific fields
		wr._pt.x = (int16)READ_LE_UINT16(pWalkRegion);
		wr._pt.y = (int16)READ_LE_UINT16(pWalkRegion + 2);
		wr._idxListIndex = READ_LE_UINT32(pWalkRegion + 4);
		wr._idxList2Index = READ_LE_UINT32(pWalkRegion + 8);

		// Region in the region data
		int size = READ_LE_UINT16(srcP);
		srcP += 2;
		wr.loadRegion(srcP, size);

		srcP += size * 4;
		_regionList.push_back(wr);
	}

	DEALLOCATE(dataP);
	DEALLOCATE(regionData);
}

/**
 * Returns the index of the walk region that contains the given point
 * @param pt		Point to locate
 * @param indexList	List of region indexes that should be ignored
 */
int WalkRegions::indexOf(const Common::Point &pt, List<int> *indexList) {
	for (uint idx = 0; idx < _regionList.size(); ++idx) {
		if ((!indexList || !indexList->contains(idx + 1)) && _regionList[idx].contains(pt))
			return idx + 1;
	}

	return -1;
}

/*--------------------------------------------------------------------------*/

void ScenePriorities::load(int resNum) {
	_resNum = resNum;
	clear();

	byte *regionData = _vm->_dataManager->getResource(RES_PRIORITY, resNum, 9999, true);
	
	if (regionData) {
		int regionCount = READ_LE_UINT16(regionData);
		for (int regionCtr = 0; regionCtr < regionCount; ++regionCtr) {
			int rlbNum = READ_LE_UINT16(regionData + regionCtr * 6 + 2);

			push_back(Region(resNum, rlbNum, RES_PRIORITY));
		}

		DEALLOCATE(regionData);
	}
}

Region *ScenePriorities::find(int priority) {
	// If no priority regions are loaded, then return the placeholder region
	if (empty())
		return &_defaultPriorityRegion;

	if (priority > 255)
		priority = 255;

	// Loop through the regions to find the closest for the givne priority level
	int minRegionId = 9998;
	Region *region = NULL;
	for (ScenePriorities::iterator i = begin(); i != end(); ++i) {
		Region *r = &(*i);
		int regionId = r->_regionId;

		if ((regionId > priority) && (regionId < minRegionId)) {
			minRegionId = regionId;
			region = r;
		}
	}

	assert(region);
	return region;
}

/*--------------------------------------------------------------------------*/

void FloatSet::add(double v1, double v2, double v3) {
	_float1 += v1;
	_float2 += v2;
	_float3 += v3;
}

void FloatSet::proc1(double v) {
	double diff = (cos(v) * _float1) - (sin(v) * _float2);
	_float2 = (sin(v) * _float1) + (cos(v) * _float2);
	_float1 = diff;
}

double FloatSet::sqrt(FloatSet &floatSet) {
	double f1Diff = _float1 - floatSet._float1;
	double f2Diff = _float2 - floatSet._float2;
	double f3Diff = _float3 - floatSet._float3;

	return ::sqrt(f1Diff * f1Diff + f2Diff * f2Diff + f3Diff * f3Diff);
}

/*--------------------------------------------------------------------------*/

GameHandler::GameHandler(): EventHandler() { 
	_nextWaitCtr = 1;
	_waitCtr.setCtr(1);
	_field14 = 10;
}

GameHandler::~GameHandler() {
	if (_globals)
		_globals->_game.removeHandler(this);
}

void GameHandler::execute() {
	if (_waitCtr.decCtr() == 0) {
		_waitCtr.setCtr(_nextWaitCtr);
		dispatch();
	}
}

void GameHandler::synchronise(Serialiser &s) {
	_lockCtr.synchronise(s);
	_waitCtr.synchronise(s);
	s.syncAsSint16LE(_nextWaitCtr);
	s.syncAsSint16LE(_field14);
}

/*--------------------------------------------------------------------------*/

SceneHandler::SceneHandler() {
	_saveGameSlot = -1;
	_loadGameSlot = -1;
}

void SceneHandler::registerHandler() {
	postInit();
	_globals->_game.addHandler(this);
}

void SceneHandler::postInit(SceneObjectList *OwnerList) {
	_delayTicks = 2;

	_globals->_scenePalette.loadPalette(0);
	_globals->_scenePalette.refresh();

	// TODO: Bunch of other scene related setup goes here
	_globals->_soundManager.postInit();

	// Set some default flags and cursor
	_globals->setFlag(12);
	_globals->setFlag(34);
	_globals->_events.setCursor(CURSOR_WALK);

	// Set the screen to scroll in response to the player moving off-screen
	_globals->_scrollFollower = &_globals->_player;

	// Set the object's that will be in the player's inventory by default
	_globals->_inventory._stunner._sceneNumber = 1;
	_globals->_inventory._scanner._sceneNumber = 1;
	_globals->_inventory._ring._sceneNumber = 1;

	// Currently hardcoded for first game room. Should be scene 1000 for title screen
	_globals->_sceneManager.setNewScene(30);	
}

void SceneHandler::process(Event &event) {
	// Main keypress handler
	if ((event.eventType == EVENT_KEYPRESS) && !event.handled) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_F1:
			// F1 - Help
			_globals->_events.setCursor(CURSOR_ARROW);
			MessageDialog::show(HELP_MSG, OK_BTN_STRING);
			break;

		case Common::KEYCODE_F2: {
			// F2 - Sound Options
			ConfigDialog *dlg = new ConfigDialog();
			dlg->runModal();
			delete dlg;
			_globals->_events.setCursorFromFlag();
			break;
		}

		case Common::KEYCODE_F3:
			// F3 - Quit
			_globals->_game.quitGame();
			event.handled = false;
			break;
			
		case Common::KEYCODE_F4:
			// F4 - Restart
			_globals->_game.restartGame();
			_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F7:
			// F7 - Restore
			_globals->_game.restoreGame();
			_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F10:
			// F10 - Pause
			GfxDialog::setPalette();
			MessageDialog::show(GAME_PAUSED_MSG, OK_BTN_STRING);
			_globals->_events.setCursorFromFlag();
			break;

		default:
			break;
		}

		_globals->_events.setCursorFromFlag();
	}

	// Check for displaying right-click dialog
	if ((event.eventType == EVENT_BUTTON_DOWN) && (event.btnState == BTNSHIFT_RIGHT) && 
			_globals->_player._uiEnabled) {
		RightClickDialog *dlg = new RightClickDialog();
		dlg->execute();
		delete dlg;

		event.handled = true;
		return;
	}

	// If there is an active scene, pass the event to it
	if (_globals->_sceneManager._scene)
		_globals->_sceneManager._scene->process(event);

	if (!event.handled) {
		// Separate check for F5 - Save key
		if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_F5)) {
			// F5 - Save
			_globals->_game.saveGame();
			event.handled = true;
			_globals->_events.setCursorFromFlag();
		}

		// Check for debugger
		if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_d) &&
			(event.kbd.flags & Common::KBD_CTRL)) {
			// Attach to the debugger
			_vm->_debugger->attach();
			_vm->_debugger->onFrame();
		}

		// Mouse press handling
		if (_globals->_player._uiEnabled && (event.eventType == EVENT_BUTTON_DOWN) && 
				!_globals->_sceneItems.empty()) {
			// Scan the item list to find one the mouse is within
			List<SceneItem *>::iterator i = _globals->_sceneItems.begin();
			while ((i != _globals->_sceneItems.end()) && !(*i)->contains(event.mousePos))
				++i;

			if (i != _globals->_sceneItems.end()) {
				// Pass the action to the item
				(*i)->doAction(_globals->_events.getCursor());
				event.handled = _globals->_events.getCursor() != CURSOR_WALK;

				if (_globals->_player._uiEnabled && _globals->_player._canWalk &&
						(_globals->_events.getCursor() != CURSOR_LOOK)) {
					_globals->_events.setCursor(CURSOR_WALK);
				} else if (_globals->_player._canWalk && (_globals->_events.getCursor() != CURSOR_LOOK)) {
					_globals->_events.setCursor(CURSOR_WALK);
				} else if (_globals->_player._uiEnabled && (_globals->_events.getCursor() != CURSOR_LOOK)) {
					_globals->_events.setCursor(CURSOR_USE);
				}
			}

			// Handle player processing
			_globals->_player.process(event);
		}
	}
}

void SceneHandler::dispatch() {
	// Handle game saving and loading
	if (_saveGameSlot != -1) {
		int saveSlot = _saveGameSlot;
		_saveGameSlot = -1;
		if (_saver->save(saveSlot, _saveName) != Common::kNoError)
			GUIErrorMessage(SAVE_ERROR_MSG);
	}
	if (_loadGameSlot != -1) {
		int loadSlot = _loadGameSlot;
		_loadGameSlot = -1;
		_saver->restore(loadSlot);
		_globals->_events.setCursorFromFlag();
	}

	_globals->_soundManager.dispatch();
	_globals->_scenePalette.signalListeners();

	// Dispatch to any objects registered in the scene
	_globals->_sceneObjects->recurse(SceneHandler::handleListener);

	// If a scene is active, then dispatch to it
	if (_globals->_sceneManager._scene) 
		_globals->_sceneManager._scene->dispatch();

	//TODO: Figure out purpose of the given list
	//_globals->_regions.forEach(SceneHandler::handleListener);	

	Event event;
	while (_globals->_events.getEvent(event))
		process(event);

	_globals->_sceneManager.checkScene();
	_globals->_sceneObjects->draw();

	_vm->_debugger->onFrame();

	// Delay between frames
	_globals->_events.delay(_delayTicks);
}

void SceneHandler::handleListener(EventHandler *obj) {
	obj->dispatch();
}

void SceneHandler::saveListener(Serialiser &ser) {
	warning("TODO: SceneHandler::saveListener");
}

/*--------------------------------------------------------------------------*/

void Game::execute() {
	// Main game loop
	bool activeFlag = false;
	do {
		// Process all currently atcive game handlers
		activeFlag = false;
		for (List<GameHandler *>::iterator i = _handlers.begin(); i != _handlers.end(); ++i) {
			GameHandler *gh = *i;
			if (gh->_lockCtr.getCtr() == 0) {
				gh->execute();
				activeFlag = true;
			}
		}
	} while (activeFlag && !_vm->getEventManager()->shouldQuit());
}

void Game::restartGame() {
	if (MessageDialog::show(RESTART_MSG, CANCEL_BTN_STRING, RESTART_BTN_STRING) == 1)
		_globals->_game.restart();
}

void Game::saveGame() {
	if (_globals->getFlag(50))
		MessageDialog::show(SAVING_NOT_ALLOWED_MSG, OK_BTN_STRING);
	else {
		// Show the save dialog
		handleSaveLoad(true, _globals->_sceneHandler._saveGameSlot, _globals->_sceneHandler._saveName);
	}
}

void Game::restoreGame() {
	if (_globals->getFlag(50))
		MessageDialog::show(RESTORING_NOT_ALLOWED_MSG, OK_BTN_STRING);
	else {
		// Show the load dialog
		handleSaveLoad(false, _globals->_sceneHandler._loadGameSlot, _globals->_sceneHandler._saveName);
	}
}

void Game::quitGame() {
	if (MessageDialog::show(QUIT_CONFIRM_MSG, CANCEL_BTN_STRING, QUIT_BTN_STRING) == 1)
		_vm->quitGame();
}

void Game::handleSaveLoad(bool saveFlag, int &saveSlot, Common::String &saveName) {
	const EnginePlugin *plugin = 0;
	EngineMan.findGame(_vm->getGameId(), &plugin);
	GUI::SaveLoadChooser *dialog;
	if (saveFlag)
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"));
	else
		dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"));

	dialog->setSaveMode(saveFlag);

	saveSlot = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
	saveName = dialog->getResultString();

	delete dialog;
}

void Game::restart() {
	_globals->_scenePalette.clearListeners();
	_globals->_soundHandler.proc3();
	
	// Reset the flags
	_globals->reset();
	_globals->setFlag(34);

	// Clear save/load slots
	_globals->_sceneHandler._saveGameSlot = -1;
	_globals->_sceneHandler._loadGameSlot = -1;

	_globals->_stripNum = 0;
	_globals->_events.setCursor(CURSOR_WALK);

	// Reset item properties
	_globals->_inventory._stunner._sceneNumber = 1;
	_globals->_inventory._scanner._sceneNumber = 1;
	_globals->_inventory._stasisBox._sceneNumber = 5200;
	_globals->_inventory._infoDisk._sceneNumber = 40;
	_globals->_inventory._stasisNegator._sceneNumber = 0;
	_globals->_inventory._keyDevice._sceneNumber = 0;
	_globals->_inventory._medkit._sceneNumber = 2280;
	_globals->_inventory._ladder._sceneNumber = 4100;
	_globals->_inventory._rope._sceneNumber = 4150;
	_globals->_inventory._key._sceneNumber = 7700;
	_globals->_inventory._translator._sceneNumber = 2150;
	_globals->_inventory._paper._sceneNumber = 7700;
	_globals->_inventory._waldos._sceneNumber = 0;
	_globals->_inventory._ring._sceneNumber = 1;
	_globals->_inventory._stasisBox2._sceneNumber = 8100;
	_globals->_inventory._cloak._sceneNumber = 9850;
	_globals->_inventory._tunic._sceneNumber = 9450;
	_globals->_inventory._candle._sceneNumber = 9500;
	_globals->_inventory._straw._sceneNumber = 9400;
	_globals->_inventory._scimitar._sceneNumber = 9850;
	_globals->_inventory._sword._sceneNumber = 9850;
	_globals->_inventory._helmet._sceneNumber = 9500;
	_globals->_inventory._items._sceneNumber = 4300;
	_globals->_inventory._concentrator._sceneNumber = 4300;
	_globals->_inventory._nullifier._sceneNumber = 4300;
	_globals->_inventory._peg._sceneNumber = 4045;
	_globals->_inventory._vial._sceneNumber = 5100;
	_globals->_inventory._jacket._sceneNumber = 9850;
	_globals->_inventory._tunic2._sceneNumber = 9850;
	_globals->_inventory._bone._sceneNumber = 5300;
	_globals->_inventory._jar._sceneNumber = 7700;
	_globals->_inventory._emptyJar._sceneNumber = 7700;

	// Change to the first game scene
	_globals->_sceneManager.changeScene(30);
}

void Game::endGame(int resNum, int lineNum) {
	_globals->_events.setCursor(CURSOR_WALK);
	Common::String msg = _vm->_dataManager->getMessage(resNum, lineNum);
	bool savesExist = _saver->savegamesExist();

	if (!savesExist) {
		// No savegames exist, so prompt the user to restart or quit
		if (MessageDialog::show(msg, QUIT_BTN_STRING, RESTART_BTN_STRING) == 0)
			_vm->quitGame();
		else
			restart();
	} else {
		// Savegames exist, so prompt for Restore/Restart
		bool breakFlag;
		do {
			if (MessageDialog::show(msg, RESTART_BTN_STRING, RESTORE_BTN_STRING) == 0) {
				breakFlag = true;
			} else {
				handleSaveLoad(false, _globals->_sceneHandler._loadGameSlot, _globals->_sceneHandler._saveName);
				breakFlag = _globals->_sceneHandler._loadGameSlot > 0;
			}
		} while (!breakFlag);
	}

	_globals->_events.setCursorFromFlag();
}

} // End of namespace tSage
