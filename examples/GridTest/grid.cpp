/*
 * @Description: 4x4 Grid Library Implementation for Qurospad
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
    
    // セル配列を初期化
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        _cells[i].isActive = false;
        _cells[i].isTouched = false;
        _cells[i].fillColor = _inactiveColor;
        _cells[i].lineColor = _gridLineColor;
    }
}

// 初期化関数
void Grid4x4::init(int16_t startX, int16_t startY, int16_t cellWidth, int16_t cellHeight) {
    _startX = startX;
    _startY = startY;
    _cellWidth = cellWidth;
    _cellHeight = cellHeight;
    
    // セル配列を再初期化
    for (int i = 0; i < GRID_TOTAL_CELLS; i++) {
        _cells[i].isActive = false;
        _cells[i].isTouched = false;
        _cells[i].fillColor = _inactiveColor;
        _cells[i].lineColor = _gridLineColor;
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
        _cells[index].isActive = active;
        _cells[index].fillColor = active ? _activeColor : _inactiveColor;
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
    
    for (int16_t row = 0; row < GRID_ROWS; row++) {
        for (int16_t col = 0; col < GRID_COLS; col++) {
            if (isPointInCell(touchX, touchY, row, col)) {
                int16_t index = getCellIndex(row, col);
                
                if (isPressed) {
                    // タッチ開始
                    _cells[index].isTouched = true;
                    
                    if (_touchMode == TOUCH_MODE_TOGGLE) {
                        // 切り替えモード：タッチでアクティブ/非アクティブを切り替え
                        _cells[index].isActive = !_cells[index].isActive;
                        _cells[index].fillColor = _cells[index].isActive ? _activeColor : _inactiveColor;
                    } else {
                        // ホールドモード：タッチ中のみアクティブ
                        _cells[index].isActive = true;
                        _cells[index].fillColor = _activeColor;
                    }
                    handled = true;
                } else {
                    // タッチ終了
                    _cells[index].isTouched = false;
                    
                    if (_touchMode == TOUCH_MODE_HOLD) {
                        // ホールドモード：タッチ終了で非アクティブに戻す
                        _cells[index].isActive = false;
                        _cells[index].fillColor = _inactiveColor;
                    }
                    handled = true;
                }
                break;
            }
        }
        if (handled) break;
    }
    
    return handled;
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
    
    // セルの内部を塗りつぶし
    _gfx->fillRect(cellX, cellY, _cellWidth, _cellHeight, _cells[index].fillColor);
}

void Grid4x4::redraw() {
    draw();
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
