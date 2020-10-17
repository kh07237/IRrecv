#include "M5Atom.h"

#define DATA_LEN	16+2					// リモコンコードのデータ長(byte)+2
#define STRING_LEN	8						// 表示用の文字列最大値
#define BUTTON_PIN	19						// リモコン形式の変更用ボタン(Analog 5+14)
#define OUTLET 32               //出力制御ピン

#if 1
#define IRCMD_ON1 0x81     //コンロON 
#define IRCMD_ON2 0x82     //グリルON 
#define IRCMD_OFF1 0x8C   //コンロOFF
#define IRCMD_OFF2 0x8C //グリルOFF
#define WAIT_COUNT_MAX 4000 //  60秒/15ms
#else //Debug
#define IRCMD_ON1 0x45     //コンロ・グリルON 
#define IRCMD_ON2 0x48     //コンロ・グリルON 
#define IRCMD_OFF1 0x46   //コンロOFF
#define IRCMD_OFF2 0x47   //グリルOFF
#define WAIT_COUNT_MAX 1000 //  15秒/15ms
#endif


// enum IR_TYPE{ AEHA=0, NEC=1, SIRC=2 };	// 家製協AEHA、NEC、SONY SIRC切り換え
#define AEHA        0
#define NEC         1
#define SIRC        2

int state = 0;  //0:OFF 1:OFF待ち　2:ON
int wait_count = 0;
int led_state = 0;

void LedOn(int color) //1:R 2:G 3:B
{
  switch(color){
    case 1:
      M5.dis.drawpix(0, 0x0000f0);
      led_state = color;
      break;
    case 2:
      M5.dis.drawpix(0, 0xf00000);
      led_state = color;
      break;
    case 3:
      M5.dis.drawpix(0, 0xf00000);
      led_state = color;
      break;
      
  }
 
}

void LedOff()
{
  M5.dis.drawpix(0, 0);
  led_state = 0;
}

void LedToggle()
{
  if(led_state == 0){
    LedOn(2);
  }else{
    LedOff();
  }  
}


void setup() {
  M5.begin(true, false, true);
	//Serial.begin(115200);	
	Serial.println("IR Tester by Wataru Kunino");
  pinMode(OUTLET, OUTPUT );
  //	ir_init();
	ir_read_init();
}

void loop(){
	byte i,len8;
	int len=0;											// 信号長
	byte data[DATA_LEN];
	char s[STRING_LEN];
  bool bButtonPressed = false;
  
	/* 赤外線リモコンコードの読み取り */
	//Serial.print("Waiting for ");
	//print_IrType(NEC);
	do{  //メインループ
    M5.update();
		if (M5.Btn.wasPressed()){
      Serial.println("ButtonPressed\n");
      data[2] = 0;
      bButtonPressed = true;
      break;
		}
    if(state == 1){
      //OFF待ち処理
      wait_count++;
      Serial.printf("wait_count = %d\n",wait_count);
      //LED点滅
      if(wait_count % 50 == 0){
        LedToggle();
      }
      //時間判定、OFF
      if(wait_count >= WAIT_COUNT_MAX){
        wait_count = 0;
        //ボタン押下と同じ制御をする
        data[2] = 0;
        bButtonPressed = true;
        state = 0;
        break;
      }
    }
    
		len = ir_read(data, DATA_LEN, NEC);
		if(len==-2)Serial.println("SYNC ERROR");
	}while( len <= 0 );

  if(bButtonPressed){ //メインループの中でボタンが押されていたら
      //OFF処理
      Serial.println("ButtonPressed2\n");
      digitalWrite(OUTLET, 0);
      state = 0;
      LedOff();
      bButtonPressed = false;
  
  }else{
    //IR受信コードに応じた処理
  	len8 = (byte)(len/8);
  	if(len%8 != 0) len8++;
  	
  	/* 結果表示 */
    if(len == 32){
    	Serial.println("Recieved.");
    	Serial.print("data[");
    	Serial.print(len8,DEC);
    	Serial.print("] = {");
    	for(i=0; i<len8 ; i++){
    		if(i>0) Serial.print(',');
    		sprintf(s,"0x%02X",data[i]);
    		Serial.print(s);
    	}
    	Serial.println("};");
    	Serial.print("length  = ");
    	Serial.println(len,DEC);
    } 
    if(data[2] == IRCMD_ON1 || data[2] == IRCMD_ON2) { 
      digitalWrite(OUTLET, 1);
      LedOn(2);
      state = 2;
      wait_count = 0;
    }
    if(data[2] == IRCMD_OFF1 || data[2] == IRCMD_OFF2) { 
      if(state == 2){
        state = 1;
      }
    }
  }  
    	/* テスト送信(バイト単位データのみ) */
    //	delay(1000);
    //	ir_send( data , len8 , type );
}

void print_IrType(const byte type){
	Serial.print("IR type = ");
	switch(type){
		case NEC:   Serial.println("NEC  "); break;
		case SIRC:  Serial.println("SIRC "); break;
		case AEHA:
		default:    Serial.println("AEHA "); break;
	}
}
