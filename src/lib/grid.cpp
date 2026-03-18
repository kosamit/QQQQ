/*
 * @Description: 4x4 Grid Library Implementation for QQQQ
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "grid.h"

// コンストラクタ
Grid4x4::Grid4x4(Arduino_GFX* gfx) {
    _gfx = gfx;
    _startX = 0;
    _startY = 0;
    _cellWidth = 50;
    _cellHeight = 50;
    _lineThickness = 2;
    _gridLineColor = DEFAULT_GRID_LINE_COLOR;
    _gridFillColor = DEFAULT_GRID_FILL_COLOR;
    _activeColor = DEFAULT_ACTIVE_COLOR;
    _inactiveColor = DEFAULT_INACTIVE_COLOR;
    _touchMode = TOUCH_MODE_TOGGLE;
    
    // タッチ状態追跡変数を初期化
    _lastTouchState = false;
    _lastTouchedRow = -1;
    _lastTouchedCol = -1;
    
    // セル配列を初期化
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        _cells[i].isActive = false;
        _cells[i].isTouched = false;
        _cells[i].needsRedraw = false;
        _cells[i].fillColor = _inactiveColor;
        _cells[i].lineColor = _gridLineColor;
        _cells[i].midiNote = 60 + i;  // デフォルトはC4から
    }
}

// 初期化関数
void Grid4x4::init(int16_t startX, int16_t startY, int16_t cellWidth, int16_t cellHeight) {
    _startX = startX;
    _startY = startY;
    _cellWidth = cellWidth;
    _cellHeight = cellHeight;
    
    // タッチ状態追跡変数を初期化
    _lastTouchState = false;
    _lastTouchedRow = -1;
    _lastTouchedCol = -1;
    
    // セル配列を再初期化
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        _cells[i].isActive = false;
        _cells[i].isTouched = false;
        _cells[i].needsRedraw = false;
        _cells[i].fillColor = _inactiveColor;
        _cells[i].lineColor = _gridLineColor;
        _cells[i].midiNote = 60 + i;  // デフォルトはC4から
    }
}

// 設定関数
void Grid4x4::setLineThickness(int16_t thickness) {
    _lineThickness = thickness;
}

void Grid4x4::setGridLineColor(uint16_t color) {
    _gridLineColor = color;
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        _cells[i].lineColor = color;
    }
}

void Grid4x4::setGridFillColor(uint16_t color) {
    _gridFillColor = color;
}

void Grid4x4::setActiveColor(uint16_t color) {
    _activeColor = color;
    // アクティブなセルの色を更新
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        if (_cells[i].isActive) {
            _cells[i].fillColor = color;
        }
    }
}

void Grid4x4::setInactiveColor(uint16_t color) {
    _inactiveColor = color;
    // 非アクティブなセルの色を更新
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        if (!_cells[i].isActive) {
            _cells[i].fillColor = color;
        }
    }
}

void Grid4x4::setTouchMode(TouchMode mode) {
    _touchMode = mode;
}

// セル操作関数
void Grid4x4::setCellActive(int16_t row, int16_t col, bool active) {
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
        int16_t index = getCellIndex(row, col);
        if (_cells[index].isActive != active) {
            _cells[index].isActive = active;
            _cells[index].fillColor = active ? _activeColor : _inactiveColor;
            _cells[index].needsRedraw = true;  // 再描画が必要
        }
    }
}

void Grid4x4::setCellTouched(int16_t row, int16_t col, bool touched) {
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
        int16_t index = getCellIndex(row, col);
        _cells[index].isTouched = touched;
    }
}

void Grid4x4::toggleCellActive(int16_t row, int16_t col) {
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
        int16_t index = getCellIndex(row, col);
        _cells[index].isActive = !_cells[index].isActive;
        _cells[index].fillColor = _cells[index].isActive ? _activeColor : _inactiveColor;
        _cells[index].needsRedraw = true;  // 再描画が必要
    }
}

bool Grid4x4::isCellActive(int16_t row, int16_t col) const {
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
        int16_t index = getCellIndex(row, col);
        return _cells[index].isActive;
    }
    return false;
}

bool Grid4x4::isCellTouched(int16_t row, int16_t col) const {
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
        int16_t index = getCellIndex(row, col);
        return _cells[index].isTouched;
    }
    return false;
}

// タッチ処理関数
bool Grid4x4::handleTouch(int16_t touchX, int16_t touchY, bool isPressed) {
    bool handled = false;
    int16_t currentRow = -1;
    int16_t currentCol = -1;
    
    // 現在のタッチ位置がどのセルにあるかを確認
    for (int16_t row = 0; row < GRID_ROWS; row++) {
        for (int16_t col = 0; col < GRID_COLS; col++) {
            if (isPointInCell(touchX, touchY, row, col)) {
                currentRow = row;
                currentCol = col;
                handled = true;
                break;
            }
        }
        if (handled) break;
    }
    
    if (handled) {
        int16_t index = getCellIndex(currentRow, currentCol);
        
        if (isPressed) {
            // タッチ開始
            _cells[index].isTouched = true;
            
            // 前回タッチしていない、または異なるセルの場合のみ切り替え
            if (!_lastTouchState || _lastTouchedRow != currentRow || _lastTouchedCol != currentCol) {
                if (_touchMode == TOUCH_MODE_TOGGLE) {
                    // 切り替えモード：タッチでアクティブ/非アクティブを切り替え
                    _cells[index].isActive = !_cells[index].isActive;
                    _cells[index].fillColor = _cells[index].isActive ? _activeColor : _inactiveColor;
                    _cells[index].needsRedraw = true;  // 再描画が必要
                } else {
                    // ホールドモード：タッチ中のみアクティブ
                    
                    // 前回のセルを非アクティブに（スライドで別のセルに移動した場合）
                    if (_lastTouchState && _lastTouchedRow >= 0 && _lastTouchedCol >= 0 &&
                        (_lastTouchedRow != currentRow || _lastTouchedCol != currentCol)) {
                        int16_t lastIndex = getCellIndex(_lastTouchedRow, _lastTouchedCol);
                        _cells[lastIndex].isActive = false;
                        _cells[lastIndex].fillColor = _inactiveColor;
                        _cells[lastIndex].needsRedraw = true;
                    }
                    
                    // 現在のセルをアクティブに
                    _cells[index].isActive = true;
                    _cells[index].fillColor = _activeColor;
                    _cells[index].needsRedraw = true;  // 再描画が必要
                }
            }
            
            // 現在のタッチ状態を記録
            _lastTouchState = true;
            _lastTouchedRow = currentRow;
            _lastTouchedCol = currentCol;
        } else {
            // タッチ終了
            _cells[index].isTouched = false;
            
            if (_touchMode == TOUCH_MODE_HOLD) {
                // ホールドモード：タッチ終了で非アクティブに戻す
                _cells[index].isActive = false;
                _cells[index].fillColor = _inactiveColor;
                _cells[index].needsRedraw = true;  // 再描画が必要
            }
            // TOGGLEモードの場合は、タッチ開始時のみトグルするため、ここでは何もしない
            
            // タッチ状態をリセット
            _lastTouchState = false;
            _lastTouchedRow = -1;
            _lastTouchedCol = -1;
        }
    } else {
        // グリッド外のタッチの場合
        if (_lastTouchState && isPressed) {
            // HOLDモードで、タッチしたままグリッド外に出た場合
            if (_touchMode == TOUCH_MODE_HOLD && _lastTouchedRow >= 0 && _lastTouchedCol >= 0) {
                // 前回アクティブだったセルを非アクティブに戻す
                int16_t lastIndex = getCellIndex(_lastTouchedRow, _lastTouchedCol);
                _cells[lastIndex].isActive = false;
                _cells[lastIndex].fillColor = _inactiveColor;
                _cells[lastIndex].needsRedraw = true;
            }
            
            // タッチ状態をリセット
            _lastTouchState = false;
            _lastTouchedRow = -1;
            _lastTouchedCol = -1;
        }
    }
    
    return handled;
}

// マルチタッチ対応の処理（シンプル版・高速化）
void Grid4x4::handleMultiTouch(int16_t* touchX, int16_t* touchY, uint8_t fingerCount) {
    // 現在タッチされているセルを記録
    bool cellTouched[GRID_TOTAL_CELLS] = {false};
    
    for (uint8_t i = 0; i < fingerCount && i < 5; i++) {
        if (touchX[i] > 0 && touchY[i] > 0) {
            // タッチ位置がどのセルか判定
            for (int16_t row = 0; row < GRID_ROWS; row++) {
                for (int16_t col = 0; col < GRID_COLS; col++) {
                    if (isPointInCell(touchX[i], touchY[i], row, col)) {
                        int16_t index = getCellIndex(row, col);
                        if (!cellTouched[index]) {
                            cellTouched[index] = true;
                        }
                        break;
                    }
                }
            }
        }
    }
    
    if (_touchMode == TOUCH_MODE_HOLD) {
        // HOLDモード：タッチされているセルだけをアクティブにする
        for (int16_t i = 0; i < GRID_TOTAL_CELLS; i++) {
            bool shouldBeActive = cellTouched[i];
            
            if (_cells[i].isActive != shouldBeActive) {
                _cells[i].isActive = shouldBeActive;
                _cells[i].fillColor = shouldBeActive ? _activeColor : _inactiveColor;
                _cells[i].needsRedraw = true;
            }
        }
    } else {
        // TOGGLEモード：新しくタッチされたセルのみトグル
        static bool lastCellTouched[GRID_TOTAL_CELLS] = {false};
        
        for (int16_t i = 0; i < GRID_TOTAL_CELLS; i++) {
            // 新しくタッチされた（前回はタッチされていなかった）セル
            if (cellTouched[i] && !lastCellTouched[i]) {
                _cells[i].isActive = !_cells[i].isActive;
                _cells[i].fillColor = _cells[i].isActive ? _activeColor : _inactiveColor;
                _cells[i].needsRedraw = true;
            }
        }
        
        // 現在の状態を記憶
        for (int16_t i = 0; i < GRID_TOTAL_CELLS; i++) {
            lastCellTouched[i] = cellTouched[i];
        }
    }
}

void Grid4x4::clearAllTouches() {
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        _cells[i].isTouched = false;
    }
}

void Grid4x4::clearAllActive() {
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        _cells[i].isActive = false;
        _cells[i].fillColor = _inactiveColor;
    }
}

// 描画関数
void Grid4x4::draw() {
    // 各セルを描画
    for (int16_t row = 0; row < GRID_ROWS; row++) {
        for (int16_t col = 0; col < GRID_COLS; col++) {
            drawCell(row, col);
        }
    }
    
    // グリッド線を描画
    drawGridLines();
}

void Grid4x4::drawCell(int16_t row, int16_t col) {
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return;
    
    int16_t cellX = _startX + col * _cellWidth;
    int16_t cellY = _startY + row * _cellHeight;
    int16_t index = getCellIndex(row, col);
    
    // グリッド線を避けてセルの内部だけを塗りつぶし
    // 左端と上端にグリッド線があるので、その分をオフセット
    int16_t innerX = cellX + _lineThickness;
    int16_t innerY = cellY + _lineThickness;
    int16_t innerWidth = _cellWidth - _lineThickness;
    int16_t innerHeight = _cellHeight - _lineThickness;
    
    _gfx->fillRect(innerX, innerY, innerWidth, innerHeight, _cells[index].fillColor);
}

void Grid4x4::redraw() {
    draw();
}

// 変更されたセルだけを再描画
void Grid4x4::redrawChangedCells() {
    for (int16_t row = 0; row < GRID_ROWS; row++) {
        for (int16_t col = 0; col < GRID_COLS; col++) {
            int16_t index = getCellIndex(row, col);
            if (_cells[index].needsRedraw) {
                drawCell(row, col);
                _cells[index].needsRedraw = false;  // フラグをクリア
            }
        }
    }
}

// 特定のセルを再描画
void Grid4x4::redrawCell(int16_t row, int16_t col) {
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
        drawCell(row, col);
        int16_t index = getCellIndex(row, col);
        _cells[index].needsRedraw = false;  // フラグをクリア
    }
}

// セルを再描画対象としてマーク
void Grid4x4::markCellForRedraw(int16_t row, int16_t col) {
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
        int16_t index = getCellIndex(row, col);
        _cells[index].needsRedraw = true;
    }
}

// グリッド線だけを再描画（公開API）
void Grid4x4::redrawGridLines() {
    drawGridLines();
}

// ユーティリティ関数
int16_t Grid4x4::getCellRow(int16_t touchY) const {
    int16_t relativeY = touchY - _startY;
    if (relativeY < 0) return -1;
    
    int16_t row = relativeY / _cellHeight;
    return (row < GRID_ROWS) ? row : -1;
}

int16_t Grid4x4::getCellCol(int16_t touchX) const {
    int16_t relativeX = touchX - _startX;
    if (relativeX < 0) return -1;
    
    int16_t col = relativeX / _cellWidth;
    return (col < GRID_COLS) ? col : -1;
}

void Grid4x4::getCellPosition(int16_t row, int16_t col, int16_t* x, int16_t* y) const {
    if (x) *x = _startX + col * _cellWidth;
    if (y) *y = _startY + row * _cellHeight;
}

void Grid4x4::getCellSize(int16_t* width, int16_t* height) const {
    if (width) *width = _cellWidth;
    if (height) *height = _cellHeight;
}

// 状態取得関数
int16_t Grid4x4::getActiveCellCount() const {
    int16_t count = 0;
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        if (_cells[i].isActive) count++;
    }
    return count;
}

int16_t Grid4x4::getTouchedCellCount() const {
    int16_t count = 0;
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        if (_cells[i].isTouched) count++;
    }
    return count;
}

void Grid4x4::getActiveCells(int16_t* rows, int16_t* cols, int16_t maxCount) const {
    int16_t count = 0;
    for (int16_t row = 0; row < GRID_ROWS && count < maxCount; row++) {
        for (int16_t col = 0; col < GRID_COLS && count < maxCount; col++) {
            if (isCellActive(row, col)) {
                if (rows) rows[count] = row;
                if (cols) cols[count] = col;
                count++;
            }
        }
    }
}

void Grid4x4::getTouchedCells(int16_t* rows, int16_t* cols, int16_t maxCount) const {
    int16_t count = 0;
    for (int16_t row = 0; row < GRID_ROWS && count < maxCount; row++) {
        for (int16_t col = 0; col < GRID_COLS && count < maxCount; col++) {
            if (isCellTouched(row, col)) {
                if (rows) rows[count] = row;
                if (cols) cols[count] = col;
                count++;
            }
        }
    }
}

// 内部関数
int16_t Grid4x4::getCellIndex(int16_t row, int16_t col) const {
    return row * GRID_COLS + col;
}

void Grid4x4::drawGridLines() {
    // 縦線を描画
    for (int16_t col = 0; col <= GRID_COLS; col++) {
        int16_t x = _startX + col * _cellWidth;
        _gfx->fillRect(x, _startY, _lineThickness, GRID_ROWS * _cellHeight, _gridLineColor);
    }
    
    // 横線を描画
    for (int16_t row = 0; row <= GRID_ROWS; row++) {
        int16_t y = _startY + row * _cellHeight;
        _gfx->fillRect(_startX, y, GRID_COLS * _cellWidth, _lineThickness, _gridLineColor);
    }
}

bool Grid4x4::isPointInCell(int16_t x, int16_t y, int16_t row, int16_t col) const {
    int16_t cellX = _startX + col * _cellWidth;
    int16_t cellY = _startY + row * _cellHeight;
    
    return (x >= cellX && x < cellX + _cellWidth && 
            y >= cellY && y < cellY + _cellHeight);
}

// MIDI関連関数の実装
void Grid4x4::setCellMidiNote(int16_t row, int16_t col, uint8_t note) {
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
        int16_t index = getCellIndex(row, col);
        _cells[index].midiNote = note;
    }
}

uint8_t Grid4x4::getCellMidiNote(int16_t row, int16_t col) const {
    if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
        int16_t index = getCellIndex(row, col);
        return _cells[index].midiNote;
    }
    return 60;  // デフォルトはC4
}

void Grid4x4::setDefaultMidiNotes(uint8_t startNote) {
    for (int16_t row = 0; row < GRID_ROWS; row++) {
        for (int16_t col = 0; col < GRID_COLS; col++) {
            int16_t index = getCellIndex(row, col);
            // 左から右、上から下に並べる
            _cells[index].midiNote = startNote + index;
        }
    }
}
