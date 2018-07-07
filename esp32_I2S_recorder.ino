/*
Copyright (c) 2018 Kouya Aoki
Released under the MIT license
https://opensource.org/licenses/mit-license.php

Portions are:
Copyright (c) 2018 Mhage
Released under the MIT license
https://github.com/MhageGH/esp32_SoundRecorder/blob/master/LICENSE.txt
*/

#include <FS.h>					// ファイルシステム
#include <SD.h>					// SDカード制御
#include <Wire.h>				// I2C
#include <ST7032.h>			// LCD制御
#include <TwiKeypad.h>	// キーパッド制御
#include "Wav.h"
#include "I2S.h"

#define MAX_KEY_INPUT		  3
#define MAX_FILE_PATH		128

//---------------------------------------------
// 録音関係
//---------------------------------------------
//int record_time = 60;						// second
//char filename[] = "/sound.wav";

const int headerSize = 44;													// Wavファイルのヘッダサイズ
//int waveDataSize = record_time * 88000;
const int numCommunicationData = 8000;							// 1回の通信で取得する総データ量
const int numPartWavData = numCommunicationData/4;	// 1回の通信で取得するWavデータ量
byte header[headerSize];														// Wavファイルのヘッダ
char communicationData[numCommunicationData];
char partWavData[numPartWavData];
File file;
bool isInitDone = false;

//---------------------------------------------
// LCD関係
//---------------------------------------------
ST7032 lcd;

//---------------------------------------------
// キーパッド関係
//---------------------------------------------
TwiKeypad keypad;					// keypad object
const int TwiAdr = 8;			// keypad's TWI(I2C) Address
const int key_table[16] = {-1, 0, -1, -1, 7, 8, 9, -1, 4, 5, 6, -1, 1, 2, 3, -1};

void record(int record_time, String filepath){
	Serial.println("record start");
	// Wavデータサイズの設定
	int waveDataSize = record_time * 88000;
	// SDカードとの通信開始
	if (!SD.begin()) return;
	CreateWavHeader(header, waveDataSize);
	// 同名ファイルが存在する場合は消去する
	SD.remove(filepath.c_str());
	file = SD.open(filepath.c_str(), FILE_WRITE);
	if (!file) return;
	file.write(header, headerSize);
	if(!isInitDone){
		I2S_Init(I2S_MODE_RX, I2S_BITS_PER_SAMPLE_32BIT);
		isInitDone = true;
	}
	for (int j = 0; j < waveDataSize/numPartWavData; ++j) {
		I2S_Read(communicationData, numCommunicationData);
		for (int i = 0; i < numCommunicationData/8; ++i) {
			partWavData[2*i] = communicationData[8*i + 2];
			partWavData[2*i + 1] = communicationData[8*i + 3];
		}
		file.write((const byte*)partWavData, numPartWavData);
	}
	file.close();
	Serial.println("record end");
}

void setup() {
	// シリアル通信（デバッグ用）の準備
	Serial.begin(115200);
	Serial.println("setup start");

	// LCDを設定する	
	lcd.begin(8,2);									// LCD初期化
	lcd.setContrast(30);						// コントラスト設定
	
	// キーパッドを設定する
	keypad.begin(Wire,TwiAdr);			// initialize keypad.
	
	Serial.println("setup end");
}

int convert_key_to_int(int8_t key){
	return key_table[key];
}

void loop() {
	int record_time_array[3] = {};
	static int loop_count = 1;

	// キー入力を受け付ける
	lcd.setCursor(0, 0);
	lcd.print("Please");
	lcd.setCursor(0, 1);
	lcd.print("PressKey");
	for(int i = 0; i < MAX_KEY_INPUT; i++){
		int input = -1;
		while(input == -1){
			int8_t key = keypad.WaitForKey();
			input = convert_key_to_int(key);
		}
		record_time_array[i] = input;
		Serial.println("key("+String(i)+") = "+String(input));
		if(i == 0){
			lcd.clear();
			lcd.print("RecTime");
		}
		lcd.setCursor(i, 1);
		lcd.print(String(input));
	}

	int req_record_time = convert_time_array_to_int(record_time_array);
	String req_filepath =	"/record_" + String(loop_count) + ".wav";
	lcd.clear();
	lcd.print("Rec " + String(loop_count));
	lcd.setCursor(0, 1);
	lcd.print(convert_time_array_to_String(record_time_array));
	record(req_record_time, req_filepath);
	loop_count++;
}

int convert_time_array_to_int(int record_time_array[3]){
	return record_time_array[0] * 60 + record_time_array[1] * 10 + record_time_array[2];
}

String convert_time_array_to_String(int record_time_array[3]){
	return String(record_time_array[0]) + ":" + String(record_time_array[1]) + String(record_time_array[2]);
}