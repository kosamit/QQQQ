/*
 * @Description: 4x4 Grid Library Usage Example for Qurospad
 * @Author: LILYGO_L
 * @Date: 2025-02-06
 * @License: GPL 3.0
 */

#include "grid.h"
#include "common_config.h"

// グリッドオブジェクト
Grid4x4* grid;

void setup() {
    Serial.begin(115200);
    
    // ディスプレイ初期化（既存のコードを使用）
    // ここでgfxオブジェクトが初期化されていると仮定
    
    // グリッドを作成（画面中央に配置）
    grid = new Grid4x4(gfx);
    grid->init(50, 50, 60, 60);  // x=50, y=50から開始、セルサイズ60x60
    
    // グリッドの設定
    grid->setLineThickness(2);
    grid->setGridLineColor(0xFFFF);      // 白い線
    grid->setActiveColor(0x07E0);        // 緑色（アクティブ時）
    grid->setInactiveColor(0x0000);      // 黒色（非アクティブ時）
    grid->setTouchMode(TOUCH_MODE_TOGGLE); // タッチで切り替えモード
    
    // グリッドを描画
    grid->draw();
    
    Serial.println("4x4 Grid initialized");
}

void loop() {
    // タッチ処理の例
    // 実際のタッチ座標を取得（既存のタッチ処理コードを使用）
    int16_t touchX, touchY;
    bool isPressed = false;
    
    // タッチ座標の取得（CST226SEタッチセンサーから）
    if (CST226SE && CST226SE->IIC_Read_Device_State()) {
        // タッチ座標を取得する処理
        // 実際の実装では、CST226SEから座標を取得
        // ここでは例として仮の値を設定
        touchX = 100;  // 実際のタッチX座標
        touchY = 100;  // 実際のタッチY座標
        isPressed = true;  // タッチされているかどうか
        
        // グリッドでタッチを処理
        if (grid->handleTouch(touchX, touchY, isPressed)) {
            // タッチがグリッド内で処理された
            Serial.println("Grid touched!");
            
            // アクティブなセルの数を表示
            int16_t activeCount = grid->getActiveCellCount();
            Serial.print("Active cells: ");
            Serial.println(activeCount);
            
            // グリッドを再描画
            grid->redraw();
        }
    }
    
    // その他の処理
    delay(10);
}

// グリッドの使用例関数
void gridUsageExamples() {
    // 特定のセルをアクティブにする
    grid->setCellActive(0, 0, true);  // 左上のセルをアクティブ
    grid->setCellActive(1, 1, true);  // 中央のセルをアクティブ
    
    // セルの状態を確認
    bool isActive = grid->isCellActive(0, 0);
    bool isTouched = grid->isCellTouched(0, 0);
    
    // セルを切り替え
    grid->toggleCellActive(2, 2);
    
    // すべてのセルを非アクティブにする
    grid->clearAllActive();
    
    // アクティブなセルのリストを取得
    int16_t activeRows[16];
    int16_t activeCols[16];
    grid->getActiveCells(activeRows, activeCols, 16);
    
    // セルの位置を取得
    int16_t cellX, cellY;
    grid->getCellPosition(1, 1, &cellX, &cellY);
    
    // セルサイズを取得
    int16_t cellWidth, cellHeight;
    grid->getCellSize(&cellWidth, &cellHeight);
    
    // タッチモードを変更
    grid->setTouchMode(TOUCH_MODE_HOLD);  // タッチ中のみアクティブモード
}
