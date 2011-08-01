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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/loleobbase.h"
#include "kyra/screen.h"
#include "kyra/timer.h"

#include "common/system.h"

#define EOBTEXTBUFFERSIZE 2048

namespace Kyra {

TextDisplayer_Eob::TextDisplayer_Eob(LolEobBaseEngine *engine, Screen *sScreen) : _vm(engine), _screen(sScreen),
	_lineCount(0), _printFlag(false), _lineWidth(0), _numCharsTotal(0), _allowPageBreak(true),
	_numCharsLeft(0), _numCharsPrinted(0), _sjisLineBreakFlag(false), _waitButtonMode(1) {

	_dialogueBuffer = new char[EOBTEXTBUFFERSIZE];
	memset(_dialogueBuffer, 0, EOBTEXTBUFFERSIZE);

	_currentLine = new char[85];
	memset(_currentLine, 0, 85);

	_textDimData = new TextDimData[_screen->screenDimTableCount()];

	for (int i = 0; i < _screen->screenDimTableCount(); i++){
		const ScreenDim *d = _screen->getScreenDim(i);
		_textDimData[i].color1 = d->unk8;
		_textDimData[i].color2 = d->unkA;
		_textDimData[i].line = d->unkC;
		_textDimData[i].column = d->unkE;
	}

	_waitButtonSpace = 0;
}

TextDisplayer_Eob::~TextDisplayer_Eob() {
	delete[] _dialogueBuffer;
	delete[] _currentLine;
	delete[] _textDimData;
}

void TextDisplayer_Eob::setupField(int dim, bool mode) {
	setPageBreakFlag();

	_textDimData[dim].color2 = _vm->_bkgColor_1;
	_screen->setScreenDim(dim);

	if (mode)
		clearCurDim();
	else
		resetDimTextPositions(dim);
}

void TextDisplayer_Eob::resetDimTextPositions(int dim) {
	_textDimData[dim].column = 0;
	_textDimData[dim].line = 0;
}

void TextDisplayer_Eob::resetPageBreakString() {
	if (vm()->_moreStrings)
		strcpy(_pageBreakString, vm()->_moreStrings[0]);
}

void TextDisplayer_Eob::setPageBreakFlag() {
	_allowPageBreak = true;
	_lineCount = 0;
}

void TextDisplayer_Eob::removePageBreakFlag() {
	_allowPageBreak = false;
}

void TextDisplayer_Eob::displayText(char *str, ...) {
	const bool isPc98 = (_vm->gameFlags().platform == Common::kPlatformPC98);

	_printFlag = false;

	_lineWidth = 0;
	_numCharsLeft = 0;
	_numCharsPrinted = 0;

	_tempString1 = str;
	_tempString2 = 0;

	_currentLine[0] = 0;

	memset(_ctrl, 0, 3);

	char c = parseCommand();

	va_list args;
	va_start(args, str);

	const ScreenDim *sd = _screen->_curDim;
	int sdx = _screen->curDimIndex();

	bool pc98PrintFlag = (isPc98 && (sdx == 3 || sdx == 4 || sdx == 5 || sdx == 15)) ? true : false;
	uint16 charsPerLine = (sd->w << 3) / (_screen->getFontWidth() + _screen->_charWidth);

	while (c) {
		char a = tolower(_ctrl[1]);

		if (!_tempString2 && c == '%') {
			if (a == 'd') {
				strcpy(_scriptParaString, Common::String::format("%d", va_arg(args, int)).c_str());
				_tempString2 = _scriptParaString;
			} else if (a == 's') {
				_tempString2 = va_arg(args, char *);
			} else {
				break;
			}

			_ctrl[0] = _ctrl[2];
			_ctrl[2] = _ctrl[1] = 0;
			c = parseCommand();
		}

		if (isPc98) {
			uint8 cu = (uint8) c;
			if (cu >= 0xE0 || (cu > 0x80 && cu < 0xA0)) {
				_currentLine[_numCharsLeft++] = c;
				_currentLine[_numCharsLeft++] = parseCommand();
				_currentLine[_numCharsLeft] = '\0';
				_lineWidth += 8;
				if ((_textDimData[sdx].column + _lineWidth) > (sd->w << 3))
					printLine(_currentLine);
				c = parseCommand();
				continue;
			}
		}

		uint16 dv = _textDimData[sdx].column / (_screen->getFontWidth() + _screen->_charWidth);

		switch (c - 1) {
		case 0:
			printLine(_currentLine);
			textPageBreak();
			_numCharsPrinted = 0;
			break;

		case 1:
			printLine(_currentLine);
			_textDimData[sdx].color2 = parseCommand();
			break;

		case 5:
			printLine(_currentLine);
			_textDimData[sdx].color1 = parseCommand();
			break;

		case 8:
			printLine(_currentLine);
			dv = _textDimData[sdx].column / (_screen->getFontWidth() + _screen->_charWidth);
			dv = ((dv + 8) & 0xfff8) - 1;
			if (dv >= charsPerLine)
				dv = 0;
			_textDimData[sdx].column = (_screen->getFontWidth() + _screen->_charWidth) * dv;
			break;

		case 11:
			_sjisLineBreakFlag=_sjisLineBreakFlag;
			// TODO (UNUSED)
			break;

		case 12:
			if (isPc98)
				_sjisLineBreakFlag = true;
			printLine(_currentLine);
			_sjisLineBreakFlag = false;
			_lineCount++;
			_textDimData[sdx].column = 0;
			_textDimData[sdx].line++;
			break;

		case 18:
			_sjisLineBreakFlag=_sjisLineBreakFlag;
			// TODO (UNUSED)
			break;

		case 23:
			_sjisLineBreakFlag=_sjisLineBreakFlag;
			// TODO (UNUSED)
			break;

		case 24:
			_sjisLineBreakFlag=_sjisLineBreakFlag;
			// TODO (UNUSED)
			break;

		case 26:
			_sjisLineBreakFlag=_sjisLineBreakFlag;
			// TODO (UNUSED)
			break;

		case 28:
			_sjisLineBreakFlag=_sjisLineBreakFlag;
			// TODO (UNUSED)
			break;

		default:
			_lineWidth += (pc98PrintFlag ? 4 : _screen->getCharWidth((uint8)c));
			_currentLine[_numCharsLeft++] = c;
			_currentLine[_numCharsLeft] = 0;

			if ((_textDimData[sdx].column + _lineWidth) > (sd->w << 3))
				printLine(_currentLine);
		}

		c = parseCommand();
	}

	va_end(args);

	if (_numCharsLeft)
		printLine(_currentLine);
}

char TextDisplayer_Eob::parseCommand() {
	if (!_ctrl[1])
		readNextPara();

	char res = _ctrl[1];
	_ctrl[1] = _ctrl[2];
	_ctrl[2] = 0;

	if (!_ctrl[1])
		readNextPara();

	return res;
}

void TextDisplayer_Eob::readNextPara() {
	char d = 0;

	if (_tempString2) {
		if (*_tempString2) {
			d = *_tempString2++;
		} else {
			_tempString2 = 0;
			d = _ctrl[0];
		}
	}

	if (!d && _tempString1) {
		if (*_tempString1)
			d = *_tempString1++;
		else
			_tempString1 = 0;
	}

	_ctrl[1] = d;
	_ctrl[2] = 0;
}

void TextDisplayer_Eob::printLine(char *str) {
	const bool isPc98 = (_vm->gameFlags().platform == Common::kPlatformPC98);
	const ScreenDim *sd = _screen->_curDim;
	int sdx = _screen->curDimIndex();
	bool pc98PrintFlag = (isPc98 && (sdx == 3 || sdx == 4 || sdx == 5 || sdx == 15)) ? true : false;

	int fh = (_screen->_currentFont == Screen::FID_SJIS_FNT) ? 9 : (_screen->getFontHeight() + _screen->_charOffset);
	int lines = (sd->h - _screen->_charOffset) / fh;

	while (_textDimData[sdx].line >= lines) {
		if ((lines - _waitButtonSpace) <= _lineCount && _allowPageBreak) {
			_lineCount = 0;
			textPageBreak();
			_numCharsPrinted = 0;
		}

		int h1 = ((sd->h / fh) - 1) * fh;
		int h2 = sd->h - fh;

		if (h2)
			_screen->copyRegion(sd->sx << 3, sd->sy + fh, sd->sx << 3, sd->sy, sd->w << 3, h2, _screen->_curPage, _screen->_curPage, Screen::CR_NO_P_CHECK);

		_screen->fillRect(sd->sx << 3, sd->sy + h1, ((sd->sx + sd->w) << 3) - 1, sd->sy + sd->h - 1, _textDimData[sdx].color2);
		if (_textDimData[sdx].line)
			_textDimData[sdx].line--;
	}

	int x1 = (sd->sx << 3) + _textDimData[sdx].column;
	int y = sd->sy + (pc98PrintFlag ? (_textDimData[sdx].line << 3) : (fh * _textDimData[sdx].line));
	int w = sd->w << 3;
	int lw = _lineWidth;
	int s = _numCharsLeft;
	char c = 0;

	if (pc98PrintFlag) {
		bool ct = true;

		if ((lw + _textDimData[sdx].column) > w) {
			if ((lines - 1 - (_waitButtonSpace << 1)) <= _lineCount)
				// cut off line to leave space for "MORE" button
				w -= vm()->_waitButtonReverveW;
		} else {
			if (!_sjisLineBreakFlag || (_lineCount + 1 < lines - 1))
				ct = false;
			else
				// cut off line to leave space for "MORE" button
				w -= vm()->_waitButtonReverveW;
		}

		if (ct) {
			w -= _textDimData[sdx].column;

			int n2 = 0;
			int n1 = (w / 4) - 1;

			while (n2 < n1 && n2 < s) {
				c = str[n2];
				uint8 cu = (uint8) c;
				if (cu >= 0xE0 || (cu > 0x80 && cu < 0xA0))
					n2++;
				n2++;
			}
			s = n2;
		}
	} else {
		if ((lw + _textDimData[sdx].column) > w) {
			if ((lines - 1) <= _lineCount && _allowPageBreak)
				// cut off line to leave space for "MORE" button
				w -= vm()->_waitButtonReverveW;

			w -= _textDimData[sdx].column;

			int n2 = 0;
			int n1 = s - 1;

			while (n1 > 0) {
				//cut off line after last space
				c = str[n1];

				lw -= _screen->getCharWidth((uint8)c);

				if (!n2 && lw <= w)
					n2 = n1;

				if (n2 && c == ' ') {
					s = n1;
					_printFlag = false;
					break;
				}
				n1--;
			}

			if (!n1) {
				if (_textDimData[sdx].column && !_printFlag) {
					s = lw = 0;
					_printFlag = true;
				} else {
					s = n2;
				}
			}
		}
	}

	c = str[s];
	str[s] = 0;

	uint8 col = _textDimData[sdx].color1;
	if (isPc98 && (sdx == 2 || sdx == 3 || sdx == 4 || sdx == 5 || sdx == 15)) {
		switch (_textDimData[sdx].color1) {
		case 0x88:
			col = 0x41;
			break;
		case 0x55:
			col = 0x81;
			break;
		case 0xaa:
			col = 0x21;
			break;
		case 0x99:
			col = 0xa1;
			break;
		case 0x33:
			col = 0xe1;
			break;
		case 0x18:
			col = 0x61;
			break;
		default:
			col = 1;
			break;
		}
		_screen->printText(str, x1 & ~3, (y + 8) & ~7, col, 0);
	} else {
		_screen->printText(str, x1, y, col, _textDimData[sdx].color2);
	}

	_textDimData[sdx].column += lw;
	_numCharsPrinted += strlen(str);

	str[s] = c;

	if (c == ' ')
		s++;

	if (str[s] == ' ')
		s++;

	uint32 len = strlen(&str[s]);
	for (uint32 i = 0; i < len; i++)
		str[i] = str[s + i];
	str[len] = 0;

	_numCharsLeft = strlen(str);
	_lineWidth = pc98PrintFlag ? (_numCharsLeft << 2) : _screen->getTextWidth(str);

	if (!_numCharsLeft && _textDimData[sdx].column < (sd->w << 3))
		return;

	_textDimData[sdx].column = 0;
	_textDimData[sdx].line++;
	_lineCount++;

	printLine(str);
}

void TextDisplayer_Eob::printDialogueText(int stringId, const char *pageBreakString) {
	const char * str = (const char *)(screen()->getCPagePtr(5) + READ_LE_UINT16(&screen()->getCPagePtr(5)[(stringId - 1) << 1]));
	assert (strlen(str) < EOBTEXTBUFFERSIZE);
	Common::strlcpy(_dialogueBuffer, str, EOBTEXTBUFFERSIZE);
	
	displayText(_dialogueBuffer);

	if (pageBreakString) {
		if (pageBreakString[0]) {
			strcpy(_pageBreakString, pageBreakString);
			displayWaitButton();
			resetPageBreakString();
		}
	}
}

void TextDisplayer_Eob::printDialogueText(const char *str, bool wait) {
	assert (strlen(str) < EOBTEXTBUFFERSIZE);
	Common::strlcpy(_dialogueBuffer, str, EOBTEXTBUFFERSIZE);

	strcpy(_dialogueBuffer, str);
	displayText(_dialogueBuffer);
	if (wait)
		displayWaitButton();
}

void TextDisplayer_Eob::printMessage(const char *str, int textColor, ...) {
	int tc = _textDimData[screen()->curDimIndex()].color1;

	if (textColor != -1)
		_textDimData[screen()->curDimIndex()].color1 = textColor;

	va_list args;
	va_start(args, textColor);
	vsnprintf(_dialogueBuffer, 240, str, args);
	va_end(args);

	displayText(_dialogueBuffer);

	if (vm()->game() != GI_EOB1)
		_textDimData[screen()->curDimIndex()].color1 = tc;

	if (!screen()->_curPage)
		screen()->updateScreen();
}

int TextDisplayer_Eob::clearDim(int dim) {
	int res = screen()->curDimIndex();
	screen()->setScreenDim(dim);
	_textDimData[dim].color1 = screen()->_curDim->unk8;
	_textDimData[dim].color2 = vm()->game() == GI_LOL ? screen()->_curDim->unkA : vm()->_bkgColor_1;
	clearCurDim();
	return res;
}

void TextDisplayer_Eob::clearCurDim() {
	int d = screen()->curDimIndex();
	const ScreenDim *tmp = screen()->getScreenDim(d);
	if (vm()->gameFlags().use16ColorMode) {
		screen()->fillRect(tmp->sx << 3, tmp->sy, ((tmp->sx + tmp->w) << 3) - 2, (tmp->sy + tmp->h) - 2, _textDimData[d].color2);
	} else
		screen()->fillRect(tmp->sx << 3, tmp->sy, ((tmp->sx + tmp->w) << 3) - 1, (tmp->sy + tmp->h) - 1, _textDimData[d].color2);

	_lineCount = 0;
	_textDimData[d].column = _textDimData[d].line = 0;
}

void TextDisplayer_Eob::textPageBreak() {
	if (vm()->game() != GI_LOL)
		SWAP(vm()->_dialogueButtonLabelCol1, vm()->_dialogueButtonLabelCol2);

	int cp = _screen->setCurPage(0);
	Screen::FontId cf = screen()->setFont(vm()->gameFlags().use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_6_FNT);

	if (vm()->game() == GI_LOL)
		vm()->_timer->pauseSingleTimer(11, true);

	vm()->_fadeText = false;
	int resetPortraitAfterSpeechAnim = 0;
	int updatePortraitSpeechAnimDuration = 0;

	if (vm()->_updateCharNum != -1)  {
		resetPortraitAfterSpeechAnim = vm()->_resetPortraitAfterSpeechAnim;
		vm()->_resetPortraitAfterSpeechAnim = 0;
		updatePortraitSpeechAnimDuration = vm()->_updatePortraitSpeechAnimDuration;
		if (vm()->_updatePortraitSpeechAnimDuration > 36)
			vm()->_updatePortraitSpeechAnimDuration = 36;
	}

	uint32 speechPartTime = 0;
	if (vm()->speechEnabled() && vm()->_activeVoiceFileTotalTime && _numCharsTotal)
		speechPartTime = vm()->_system->getMillis() + ((_numCharsPrinted * vm()->_activeVoiceFileTotalTime) / _numCharsTotal);

	const ScreenDim *dim = screen()->getScreenDim(screen()->curDimIndex());

	int x = ((dim->sx + dim->w) << 3) - (_vm->_dialogueButtonW + 3);
	int y = 0;
	int w = vm()->_dialogueButtonW;

	if (vm()->game() == GI_LOL) {
		if (vm()->_needSceneRestore && (vm()->_updateFlags & 2)) {
			if (vm()->_currentControlMode || !(vm()->_updateFlags & 2)) {
				y = dim->sy + dim->h - 5;
			} else {
				x += 6;
				y = dim->sy + dim->h - 2;
			}
		} else {
			y = dim->sy + dim->h - 10;
		}
	} else {
		y = vm()->_waitButtonPresY[_waitButtonMode];
		x = vm()->_waitButtonPresX[_waitButtonMode];
		w = vm()->_waitButtonPresW[_waitButtonMode];
	}

	if (vm()->gameFlags().use16ColorMode) {
		vm()->gui_drawBox(x + 8, (y & ~7) - 1, 66, 10, 0xee, 0xcc, -1);
		screen()->printText(_pageBreakString, (x + 37 - (strlen(_pageBreakString) << 1) + 4) & ~3, (y + 2) & ~7, 0xc1, 0);
	} else {
		vm()->gui_drawBox(x, y, w, vm()->_dialogueButtonH, vm()->_color1_1, vm()->_color2_1, vm()->_bkgColor_1);
		screen()->printText(_pageBreakString, x + (w >> 1) - (vm()->screen()->getTextWidth(_pageBreakString) >> 1), y + 2, vm()->_dialogueButtonLabelCol1, 0);
	}

	vm()->removeInputTop();

	bool loop = true;
	bool target = false;

	do {
		int inputFlag = vm()->checkInput(0, false) & 0xFF;
		vm()->removeInputTop();

		while (!inputFlag) {
			vm()->update();

			if (vm()->speechEnabled()) {
				if (((vm()->_system->getMillis() > speechPartTime) || (vm()->snd_updateCharacterSpeech() != 2)) && speechPartTime) {
					loop = false;
					inputFlag = vm()->_keyMap[Common::KEYCODE_RETURN];
					break;
				}
			}

			inputFlag = vm()->checkInput(0, false) & 0xFF;
			vm()->removeInputTop();
		}

		vm()->gui_notifyButtonListChanged();

		if (inputFlag == vm()->_keyMap[Common::KEYCODE_SPACE] || inputFlag == vm()->_keyMap[Common::KEYCODE_RETURN]) {
			loop = false;
		} else if (inputFlag == 199 || inputFlag == 201) {
			if (vm()->posWithinRect(vm()->_mouseX, vm()->_mouseY, x, y, x + w, y + 9)) {
				if (_vm->game() == GI_LOL)
					target = true;
				else
					loop = false;
			}
		} else if (inputFlag == 200 || inputFlag == 202) {
			if (target)
				loop = false;
		}
	} while (loop);

	if (vm()->gameFlags().use16ColorMode)
		screen()->fillRect(x + 8, y, x + 57, y + 9, _textDimData[screen()->curDimIndex()].color2);
	else
		screen()->fillRect(x, y, x + w - 1, y + 8, _textDimData[screen()->curDimIndex()].color2);

	clearCurDim();

	if (vm()->game() == GI_LOL)
		vm()->_timer->pauseSingleTimer(11, false);

	if (vm()->_updateCharNum != -1) {
		vm()->_resetPortraitAfterSpeechAnim = resetPortraitAfterSpeechAnim;
		if (updatePortraitSpeechAnimDuration > 36)
			updatePortraitSpeechAnimDuration -= 36;
		else
			updatePortraitSpeechAnimDuration >>= 1;

		vm()->_updatePortraitSpeechAnimDuration = updatePortraitSpeechAnimDuration;
	}

	screen()->setFont(cf);
	screen()->setCurPage(cp);

	if (vm()->game() != GI_LOL)
		SWAP(vm()->_dialogueButtonLabelCol1, vm()->_dialogueButtonLabelCol2);

	vm()->removeInputTop();
}

void TextDisplayer_Eob::displayWaitButton() {
	vm()->_dialogueNumButtons = 1;
	vm()->_dialogueButtonString[0] = _pageBreakString;
	vm()->_dialogueButtonString[1] = 0;
	vm()->_dialogueButtonString[2] = 0;
	vm()->_dialogueHighlightedButton = 0;

	vm()->_dialogueButtonPosX = &vm()->_waitButtonPresX[_waitButtonMode];
	vm()->_dialogueButtonPosY = &vm()->_waitButtonPresY[_waitButtonMode];
	vm()->_dialogueButtonW = vm()->_waitButtonPresW[_waitButtonMode];
	vm()->_dialogueButtonYoffs = 0;

	SWAP(_vm->_dialogueButtonLabelCol1, _vm->_dialogueButtonLabelCol2);
	vm()->drawDialogueButtons();

	if (!vm()->shouldQuit())
		vm()->removeInputTop();

	while (!vm()->processDialogue() && !vm()->shouldQuit()) {}
	
	vm()->_dialogueButtonW = 95;
	SWAP(_vm->_dialogueButtonLabelCol1, _vm->_dialogueButtonLabelCol2);
	clearCurDim();
}

} // End of namespace Kyra

#endif // (ENABLE_EOB || ENABLE_LOL)
