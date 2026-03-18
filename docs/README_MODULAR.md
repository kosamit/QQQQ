# QQQQ Modular Test Structure

このプロジェクトは、元の`Original_Test.ino`ファイルを機能ごとに分割して、より保守しやすい構造に再編成したものです。

## ファイル構造

### メインファイル

- **`Original_Test_Modular.ino`** - モジュラー版のメインファイル

### 共通ファイル

- **`common_config.h`** - 共通の設定と定義
- **`common_functions.h`** - 共通関数の宣言
- **`common_functions.cpp`** - 共通関数の実装

### テストモジュール

- **`touch_test.h` / `touch_test.cpp`** - タッチセンサーテスト
- **`vibration_test.h` / `vibration_test.cpp`** - 振動モーターテスト
- **`rtc_test.h` / `rtc_test.cpp`** - リアルタイムクロックテスト
- **`microphone_test.h` / `microphone_test.cpp`** - マイクロフォンテスト
- **`sd_test.h` / `sd_test.cpp`** - SD カードテスト
- **`wifi_test.h` / `wifi_test.cpp`** - WiFi 接続テスト
- **`music_test.h` / `music_test.cpp`** - 音楽再生テスト

## 各テストの機能

### 1. Touch Test

- タッチセンサーの動作確認
- 複数指タッチの検出
- タッチ座標の表示

### 2. Vibration Motor Test

- 振動モーターの動作確認
- 振動強度の調整（0-20 レベル）
- リアルタイム強度表示

### 3. RTC Test

- リアルタイムクロックの動作確認
- 日時表示とリセット機能
- NTP 時刻同期

### 4. Microphone Test

- マイクロフォンの動作確認
- 音声データの取得と表示
- 左右チャンネル情報

### 5. SD Test

- SD カードの動作確認
- カードタイプとサイズの表示
- ファイルシステムのテスト

### 6. WiFi Test

- WiFi 接続テスト
- ネットワークスキャン
- HTTP ファイルダウンロード
- NTP 時刻同期

### 7. Music Test

- SD カードからの音楽再生
- 音量調整（0-21 レベル）
- 再生進捗表示

## 使用方法

1. **`Original_Test_Modular.ino`**をメインファイルとして使用
2. 各テストモジュールは自動的に順次実行される
3. タッチ操作でテストの再実行やスキップが可能

## 利点

- **保守性の向上**: 各機能が独立したファイルに分離
- **再利用性**: 個別のテストモジュールを他のプロジェクトで再利用可能
- **デバッグの容易さ**: 問題のある機能を特定しやすい
- **拡張性**: 新しいテストを簡単に追加可能

## 注意事項

- 元の`Original_Test.ino`ファイルは保持されており、必要に応じて参照可能
- すべての依存関係とライブラリは元のファイルと同じものを使用
- コンパイル時にはすべての`.cpp`ファイルが自動的に含まれる
