/*
 * @Description: 4x4 Grid Library for Qurospad
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#ifndef GRID_H
#define GRID_H

#include <Arduino.h>
#include "Arduino_GFX_Library.h"

// グリッドの設定定数
#define GRID_ROWS 4
#define GRID_COLS 4
#define GRID_TOTAL_CELLS (GRID_ROWS * GRID_COLS)

// デフォルト色設定
#define DEFAULT_GRID_LINE_COLOR 0xFFFF    // 白色
#define DEFAULT_GRID_FILL_COLOR 0x0000    // 黒色
#define DEFAULT_ACTIVE_COLOR 0x07E0       // 緑色
#define DEFAULT_INACTIVE_COLOR 0x0000     // 黒色

// タッチモード設定
enum TouchMode {
    TOUCH_MODE_HOLD,      // タッチ中のみアクティブ
    TOUCH_MODE_TOGGLE     // タッチでアクティブ/非アクティブ切り替え
};

// グリッドセルの状態
struct GridCell {
    bool isActive;        // アクティブ状態
    bool isTouched;       // タッチ中かどうか
    bool needsRedraw;     // 再描画が必要かどうか
    uint16_t fillColor;   // 内部の色
    uint16_t lineColor;   // 線の色
};

class Grid4x4 {
private:
    Arduino_GFX* _gfx;                    // ディスプレイオブジェクト
    int16_t _startX, _startY;            // グリッドの左上座標
    int16_t _cellWidth, _cellHeight;     // セルの幅と高さ
    int16_t _lineThickness;              // 線の太さ
    uint16_t _gridLineColor;             // グリッド線の色
    uint16_t _gridFillColor;             // グリッド内部の色
    uint16_t _activeColor;               // アクティブ時の色
    uint16_t _inactiveColor;             // 非アクティブ時の色
    TouchMode _touchMode;                // タッチモード
    GridCell _cells[GRID_TOTAL_CELLS];   // グリッドセル配列
    
    // タッチ状態追跡用変数
    bool _lastTouchState;                // 前回のタッチ状態
    int16_t _lastTouchedRow;             // 最後にタッチされた行
    int16_t _lastTouchedCol;             // 最後にタッチされた列
    
    // 内部関数
    int16_t getCellIndex(int16_t row, int16_t col) const;
    void drawCell(int16_t row, int16_t col);
    void drawGridLines();
    bool isPointInCell(int16_t x, int16_t y, int16_t row, int16_t col) const;

public:
    // コンストラクタ
    Grid4x4(Arduino_GFX* gfx);
    
    // 初期化関数
    void init(int16_t startX, int16_t startY, int16_t cellWidth, int16_t cellHeight);
    
    // 設定関数
    void setLineThickness(int16_t thickness);
    void setGridLineColor(uint16_t color);
    void setGridFillColor(uint16_t color);
    void setActiveColor(uint16_t color);
    void setInactiveColor(uint16_t color);
    void setTouchMode(TouchMode mode);
    
    // セル操作関数
    void setCellActive(int16_t row, int16_t col, bool active);
    void setCellTouched(int16_t row, int16_t col, bool touched);
    void toggleCellActive(int16_t row, int16_t col);
    bool isCellActive(int16_t row, int16_t col) const;
    bool isCellTouched(int16_t row, int16_t col) const;
    
    // タッチ処理関数
    bool handleTouch(int16_t touchX, int16_t touchY, bool isPressed);
    void clearAllTouches();
    void clearAllActive();
    
    // 描画関数
    void draw();
    void redraw();
    void redrawChangedCells();  // 変更されたセルだけを再描画
    void redrawCell(int16_t row, int16_t col);  // 特定のセルを再描画
    void markCellForRedraw(int16_t row, int16_t col);  // セルを再描画対象としてマーク
    void redrawGridLines();  // グリッド線だけを再描画（公開API）
    
    // ユーティリティ関数
    int16_t getCellRow(int16_t touchY) const;
    int16_t getCellCol(int16_t touchX) const;
    void getCellPosition(int16_t row, int16_t col, int16_t* x, int16_t* y) const;
    void getCellSize(int16_t* width, int16_t* height) const;
    
    // 状態取得関数
    int16_t getActiveCellCount() const;
    int16_t getTouchedCellCount() const;
    void getActiveCells(int16_t* rows, int16_t* cols, int16_t maxCount) const;
    void getTouchedCells(int16_t* rows, int16_t* cols, int16_t maxCount) const;
};

#endif // GRID_H
