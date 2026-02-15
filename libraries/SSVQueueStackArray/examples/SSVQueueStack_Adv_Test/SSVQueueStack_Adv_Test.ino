 /*

Manage QUEUE-STACK storage with custom MyDataType over serial connection.
Has menu, allow to exersize sorage by to push-pop data items to storage, show storage status and data items.
Allow to change storage property on the fly. 
Also has three timers (to aytomatically push data item to storage, pop it out, by single items and ALL )

 */

#include "SSVQueueStackArray.h"

#include <SSVTimer.h>
SSVTimer TmrPush, TmrPop, TmrPopALL;

char CmdBuff [8] = {0,0,0,0,0,0,0,0};//commands like MU12
byte BuffInd = 0;

struct MyDataType
  {
  long timestamp;
  int id;
  float value;
  };

int gCNT=0;

// create a queue of characters.

//Push-T0-Full-Storage-Action: PTFSA_Resize, PTFSA_Overwrite, PTFSA_Ignore
//StorageType: QUEUE_Storage (FIFO) or STACK_Storage (LIFO)
//SSVQueueStackArray <MyDataType> storage; //no parenthesis?

SSVQueueStackArray <MyDataType> storage (STACK_Storage, //QUEUE_Storage (FIFO) or STACK_Storage (LIFO)
                                         PTFSA_Resize,  //PTFSA_Resize, PTFSA_Overwrite, PTFSA_Ignore
                                         10 );           //init size

// startup point entry (runs once).
void setup () 
{
  //timer to push data to Queue  -- disabled now
  TmrPush.SetInterval(250);
  TmrPush.SetEnabled(false);
  TmrPush.SetOnTimer(TmrPush_Func);
  
  TmrPop.SetInterval(1000);
  TmrPop.SetEnabled(false);
  TmrPop.SetOnTimer(TmrPop_Func);

  TmrPopALL.SetInterval(10000);
  TmrPopALL.SetEnabled(false);
  TmrPopALL.SetOnTimer(TmrPopALL_Func);

  // start serial communication.
  Serial.begin (115200);
  delay(1000); //for stability
  // print end of line character.
  Serial.println ();
  Proc_S(); //show status
  ShowHelp();
  Serial.print("?--> ");  
  BuffInd = 0;
}

// loop the main sketch.
void loop () 
{
  TmrPush.RefreshIt();
  TmrPop.RefreshIt();
  TmrPopALL.RefreshIt();
  serialEvent(); //check serial port
}

void PrintData(MyDataType V)
{ 
  Serial.printf("TS: %lu, ID: %d, Value: %.2f \r\n", V.timestamp, V.id, V.value);
}

MyDataType GetData()
{
  MyDataType V;
  V.timestamp = millis();
  V.id=gCNT++; //gCNT++;
  V.value=(float)random(0, 1000)/100;
  return V;
}

void TmrPop_Func()
{
  Serial.printf("Pop Timer: ");
  popDataItem();
}

void TmrPopALL_Func()
{
  ShowStorageStatus(0x01 + 0x02); //show heap size and storage status with no data
  Serial.printf("PopALL Timer: ");
  while (!storage.isEmpty()) 
    { 
    popDataItem(); 
    yield();  //to keep WDT happy
    }
  ShowStorageStatus(0x01 + 0x02); //show heap size and storage status with no data
}

void TmrPush_Func()
{
  Serial.printf("Push Timer: ");
  pushDataItem();
}

void serialEvent() 
{
  while (Serial.available()) 
  {
    // get the new byte:
    char c = (char)Serial.read();
    if ( (c==0x0D) or (c==0x0A) ) //end of command?
     { //yes, end of command
      Serial.println();
      if (BuffInd > 0) //bufer has some command?
       {//yes, buffer has some command
       CmdBuff[BuffInd]=c; //otherwise functions like strtol() may not correct work with buffer
       ProcCmd();
       BuffInd=0;
       return;
       } 
       else 
        {//no, buffer is emppty
        Serial.println("ERR: buffer is empty.");
        }
     } 
       else
        {//no, it is no end of command, 08Hex - backspace ?
        if (isPrintable(c)) //legal char for command? //isAlphaNumeric()?
          {//yes, legal char for command
          Serial.print(c);
          if ( BuffInd < sizeof(CmdBuff) ) //is buffer still has space, no end of the buffer? 
           {//yes, no end of the buffer
           CmdBuff[BuffInd]=c; 
           BuffInd++; 
           }
            else
            {//no more space in buffer
            Serial.println("ERR: command is too long.");  
            BuffInd=0; 
            while (Serial.available()) {Serial.read();} //to clear incoming buffer
            }
          }//end of "yes, legal char for command"
        }//end of "no, it is no end of command"  
  }//end of while-loop
}//end of serialEvent()

void ProcCmd()
{
  Serial.println(); 
  Serial.print("CMD:"); 
  for (byte i=0; i<BuffInd; i++) { Serial.print( (char) CmdBuff[i]); }
  Serial.println();  
  switch (toupper(CmdBuff[0]))
  {
  case '?': ShowHelp(); break;
  case 'U': Proc_U(); break; //push to Storage
  case 'O': Proc_O(); break; //pop from Storage
  case 'S': Proc_S(); break; //show Storage Status
  case 'T': Proc_T(); break; //Type of Storage
  case 'A': Proc_A(); break; //Type of PTFSAction (Resize, Overwrite or Ignore)
  case 'M': Proc_M(); break; //Timer test [DMD, ME, Mddd]
  case 'R': Proc_R(); break; //ResetData or Resize storage
  case 'N': Proc_N(); break; //Normalize
  default:  Serial.println( (byte) CmdBuff[0], HEX);  Serial.println("Unknown command. "); /*ShowHelp();*/ break;
  }
Serial.print("?--> ");  
}

void ShowHelp()
{
Serial.println();
Serial.print("Command is "); Serial.print(sizeof(CmdBuff)); Serial.println(" (max) printable non-case sensitive characters, following by CR(0x0D)");
Serial.println("List of commands: ");
Serial.println("  ?   - show this help,");
Serial.println("  S   - Show Status of Storage");
Serial.println("        S     - Storage Status only, with no content");
Serial.println("        SA    - All information (may be long)");
Serial.println("        SH    - ESP8266 HEAP size only");
Serial.println("        SD    - Storage DATA only (may be long)");
Serial.println("        ST    - Show Timers Intervals only (same as M?)");

Serial.println("  U   - Push in Data to Storage");
Serial.println("        U     - Push in Single Data Item ");
Serial.println("        Uddd  - Push in ddd (decimal) Data Items"); //not done

Serial.println("  O   - Pop Data from Storage:");
Serial.println("        O     - Pop out Single Data Item ");
Serial.println("        OA    - Pop out ALL Data Items");
Serial.println("        Oddd  - Pop out ddd (decimal) Data Items"); 

Serial.println("  M   - 3-Timer Test: PushTimer, PopTimer and PopALLTimer.");
Serial.println("        ME    - Enable Timer Test");
Serial.println("        MD    - Disable Timer Test");
Serial.println("        M?    - Show Timers Intervals");
Serial.println("        MUddd - Push-Timer interval, x0.1S");
Serial.println("        MOddd - Pop-Timer interval, x0.1S");
Serial.println("        MAddd - Pop-ALL-Timer interval, x0.1S");

Serial.println("  Tx  - Select Storage Type:");
Serial.println("        TQ - QUEUE (FIFO)");
Serial.println("        TS - STACK (LIFO)");

Serial.println("  A   - Select Push-To-Full-Storage Action");
Serial.println("        A1 - Resize");
Serial.println("        A2 - Overwrite");
Serial.println("        A3 - Ignore");

Serial.println("  R   - Reset Data or Resize Storage");
Serial.println("        R     - Reset Data");
Serial.println("        RZddd - Resize");

Serial.println("  N   - Normalize Storage");
//resetData
//resize ddd
//normalizeStorage
//show oldest and newest

//Serial.println("?-->");
}

void ShowStorageStatus(byte WhatToShow)
{
/*
0x01: show heap size only
0x02: show storage status only (no data)
0x04: show timers intervals only
0x08: show storage data only
*/
Serial.printf("--- INFO:\r\n");

if ((WhatToShow & 0x04) != 0)
  {
  Serial.printf("Push Timer Interval is: %.1fS\r\n", (float)TmrPush.GetInterval()/1000);
  Serial.printf("Pop Timer Interval is: %.1fS\r\n", (float)TmrPop.GetInterval()/1000);
  Serial.printf("PopALL Timer Interval is: %.1fS\r\n", (float)TmrPopALL.GetInterval()/1000);
  }

if ((WhatToShow & 0x02) != 0)
  {
  Serial.print("Storage Type: "); PrintStorageType(); Serial.print("\r\n");
  Serial.printf("is Empty: %s\r\n", (storage.isEmpty()) ? "YES" : "NO" ); 
  Serial.printf("is Full: %s\r\n", (storage.isFull()) ? "YES" : "NO" ); 
  Serial.printf("PTFSAction: "); PrintPTFSAction(); Serial.print("\r\n");
  Serial.printf("Count: %d\r\n", storage.getCount());
  Serial.printf("Size: %d\r\n", storage.getSize());
  if (storage.isEmpty())
    {
    Serial.println("Storage is empty, no Newest-Oldest Items in storage");
    }
    else
      {
      MyDataType V;
      V = storage.peekOldest();
      Serial.printf("Oldest Item: "); 
      PrintData(V);
      V = storage.peekNewest();
      Serial.printf("Newest Item: "); 
      PrintData(V);
      }
  }

if ((WhatToShow & 0x08) != 0)
  {
  if (storage.isEmpty())  
  {
  Serial.println("Storage is empty, no Data in storage");
  }
  else
    {
    MyDataType V;
    for (int unsigned i=0; i<storage.getCount(); i++)
      {
      V = storage.getDataByIndex(i);
      if (i==0) Serial.printf("  index [%d] (oldest): -> ", i);
           else Serial.printf("  index [%d]         : -> ", i);
      PrintData(V);
      yield();  //to keep WDT happy
      }
    }
  }  

if ((WhatToShow & 0x01) != 0) Serial.printf("ESP HEAP SIZE: %d \r\n", ESP.getFreeHeap() );  
}

void PrintStorageType()
{
  Serial.printf("%s", (storage.getStorageType() == STACK_Storage) ? "STACK (LIFO)" : "QUEUE (FIFO)" );
}

void PrintPTFSAction()
{
switch (storage.getPTFSAction())
  {
  case (PTFSA_Resize):      Serial.printf("Resize"); break;
  case (PTFSA_Overwrite):   Serial.printf("Overwrite"); break;
  default: /*PTFQA_Ignore*/ Serial.printf("Ignore"); break;
  }  
}


void Proc_A()  //Type of PTFSAction (Resize, Overwrite or Ignore)
{
if (BuffInd > 2) {Serial.println("Too long command"); return; } 
if (BuffInd < 2) {Serial.println("Too short command"); return; } 
switch (toupper(CmdBuff[1]))
  {
  case '1': 
    Serial.print("PTFSAction: "); PrintPTFSAction(); Serial.print(" --> ");
    storage.setPTFSAction(PTFSA_Resize) ; 
    PrintPTFSAction();  Serial.print("\r\n");
    break;
  case '2': 
    Serial.print("PTFSAction: "); PrintPTFSAction(); Serial.print(" --> ");
    storage.setPTFSAction(PTFSA_Overwrite) ; 
    PrintPTFSAction();  Serial.print("\r\n");
    break;
  case '3': 
    Serial.print("PTFSAction: "); PrintPTFSAction(); Serial.print(" --> ");
    storage.setPTFSAction(PTFSA_Ignore) ; 
    PrintPTFSAction();  Serial.print("\r\n");
    break;
  default:  Serial.println("Unknown \"A\" command."); /*ShowHelp();*/ break;
  }

}

void Proc_T() //TL: STACK (LIFO) or TF: QUEUE (FIFO)
{
if (BuffInd > 2) {Serial.println("Too long command"); return; } 
if (BuffInd < 2) {Serial.println("Too short command"); return; } 
switch (toupper(CmdBuff[1]))
  {
  case 'Q': 
    Serial.print("Storage Type: "); PrintStorageType(); Serial.print(" --> ");
    storage.setStorageType(QUEUE_Storage) ; 
    PrintStorageType();  Serial.print("\r\n");
    break;
  case 'S': 
    Serial.print("Storage Type: "); PrintStorageType(); Serial.print(" --> ");
    storage.setStorageType(STACK_Storage) ; 
    PrintStorageType();  Serial.print("\r\n");
    break;
  default:  Serial.println("Unknown \"T\" command."); /*ShowHelp();*/ break;
  }
}

void popDataItem()
{
  if (storage.isEmpty()) { Serial.printf("Storage is empty, can't pop item from storage.\r\n"); return;}
  MyDataType V;
  V = storage.pop();
  Serial.print("Data poped: ");
  PrintData(V);
}

void pushDataItem()
{
MyDataType V = GetData();
Serial.print("Data push: ");
PrintData(V);
storage.push(V);
}

void Proc_M() //Timer test [MD, ME, MUddd, MOddd, MAddd]
{
if (BuffInd > 5) { Serial.println("Too long command"); return; }
if (BuffInd < 2) { Serial.println("Too short command"); return; }
if ( (BuffInd == 2) && (toupper(CmdBuff[1]) == 'E') ) 
  {
    Serial.println("Timers Enabled");  
    if (TmrPush.GetInterval()>0) TmrPush.SetEnabled(true); 
    if (TmrPop.GetInterval()>0)  TmrPop.SetEnabled(true);
    if (TmrPopALL.GetInterval()>0)  TmrPopALL.SetEnabled(true);
  }
  
if ( (BuffInd == 2) && (toupper(CmdBuff[1]) == 'D') ) 
  {
    Serial.println("Timers Disabled"); 
    TmrPush.SetEnabled(false);
    TmrPop.SetEnabled(false); 
    TmrPopALL.SetEnabled(false); 
  }

if ( (BuffInd == 2) && (CmdBuff[1] == '?') ) 
  {
  Serial.printf("Push Timer Interval is: %.1fS\r\n", (float)TmrPush.GetInterval()/1000);
  Serial.printf("Pop Timer Interval is: %.1fS\r\n", (float)TmrPop.GetInterval()/1000);
  Serial.printf("PopALL Timer Interval is: %.1fS\r\n", (float)TmrPopALL.GetInterval()/1000);
  }

bool charU = (toupper(CmdBuff[1]) == 'U');
bool charO = (toupper(CmdBuff[1]) == 'O');
bool charA = (toupper(CmdBuff[1]) == 'A');

if ( (BuffInd >= 3) && (charU || charO || charA) && (isDigit(CmdBuff[2])) )
  {  //MO or MU with number
  long nn= strtol( CmdBuff + 2, 0, 10); //skip first char
  //if (nn==0) { Serial.println("Number is 0, not allowed."); return; }
  nn = nn*100; //to mS

  switch (toupper(CmdBuff[1]))
  {
    case 'U': 
    Serial.printf("Push Timer Interval is: %.1fS\r\n", (float)nn/1000);
    TmrPush.SetInterval(nn);
    if (TmrPush.GetInterval()==0) TmrPush.SetEnabled(false);
    break;

    case 'O': 
    Serial.printf("Pop Timer Interval is: %.1fS\r\n", (float)nn/1000);
    TmrPop.SetInterval(nn);
    if (TmrPop.GetInterval()==0) TmrPop.SetEnabled(false);
    break;

    case 'A': 
    Serial.printf("Pop-ALL Timer Interval is: %.1fS\r\n", (float)nn/1000);
    TmrPopALL.SetInterval(nn);
    if (TmrPopALL.GetInterval()==0) TmrPopALL.SetEnabled(false);
    break;
  }
 }
}
   

void Proc_U()  //push to Queue
{
//  U - Push in Single Data Item
//  Uddd - Push in ddd (decimal) Data Items
if (BuffInd > 4) { Serial.println("Too long command"); return; }
//if full?
if (BuffInd == 1) 
  {  //push single Data Item
  pushDataItem();
  }
if ( (BuffInd >= 2) && (isDigit(CmdBuff[1])) )
  {  //push by number
  long nn= strtol( CmdBuff + 1, 0, 10); //skip first char
  if (nn==0) { Serial.println("Number is 0, nothing to push into storage."); return; }
  for (long i=0; i<nn; i++)
    {
    pushDataItem();
    //if full?
    }
  }
Serial.println();
ShowStorageStatus(0x01 + 0x02 + 0x08); //show status
}

void Proc_O()  //Pop from Queue
{
//        O - Pop out Single Data Item
//        OA - Pop out ALL Data Items
//        Oddd - Pop out dd (decimal) Data Items
if (BuffInd > 4) { Serial.println("Too long command"); return; }
if (storage.isEmpty()) { Serial.println("Storage is empty, can't pop item from storage."); return; }

if (BuffInd == 1) 
  {  //pop single Data Item
  popDataItem();
  }
  
if ( (BuffInd == 2) && ((toupper(CmdBuff[1])) == 'A') )
  {  //pop all data items
  Serial.printf("Popped ALL %d Data Items...\r\n", storage.getCount());
  while (!storage.isEmpty()) 
    { 
    popDataItem(); 
    yield();  //to keep WDT happy
    }
  }

if ( (BuffInd >= 2) && (isDigit(CmdBuff[1])) )
  {  //pop by number
  long nn= strtol( CmdBuff + 1, 0, 10); //skip first char
  if (nn==0) { Serial.println("Number is 0, nothing to pop out from storage."); return; }
  for (long i=0; i<nn; i++)
    {
      popDataItem();
      if  (storage.isEmpty()) { Serial.println("Storage is empty, can't pop item from storage."); break;}
    }
  }
Serial.println();
ShowStorageStatus(0x01 + 0x02 + 0x08);//show status
}


//  S  - Storage Status only, with no content");
//  SA - All information (may be long)");
//  SH - ESP8266 HEAP size only");
//  SD - Storage Data only (may be long)");
//  ST - Show Timers Intervals (same as M?)");

/*
0x01: show heap size only
0x02: show storage status only (no data)
0x04: show timers intervals only
0x08: show storage data only
*/
void Proc_S()  //show Queue status
{
if (BuffInd > 2) {Serial.println("Too long command"); return; } 
if (BuffInd == 1) {ShowStorageStatus(0x02);}
if (BuffInd == 2)
  {
  switch (toupper(CmdBuff[1]))
    {
    case 'A': { ShowStorageStatus(0x0F); break;}
    case 'H': { ShowStorageStatus(0x01); break;}
    case 'D': { ShowStorageStatus(0x08); break;}
    case 'T': { ShowStorageStatus(0x04); break;}
    default: Serial.println("Unknown \"S\" command."); /*ShowHelp();*/
    }
  }
}

//  R     - Reset Data
//  RZddd - Resize
void Proc_R() //ResetData or Resize storage
{
  if (BuffInd > 5) {Serial.println("Too long command"); return; } 
  if (BuffInd == 1) {storage.resetData(); Serial.println("Data has reseted."); }

if ( (BuffInd >= 3) && (toupper(CmdBuff[1]) == 'Z')  && (isDigit(CmdBuff[2])) )
  {  //resize by number
  long nn= strtol( CmdBuff + 2, 0, 10); //skip first char
  if (nn==0) { Serial.println("Resize to Zero is not allowed."); return; }
  storage.resize(nn);
  Serial.printf("Resized to new size: %d.\r\n", nn); 
  }  
ShowStorageStatus(0x02); //show storage status, no data  
}

void Proc_N() //Normalize
{
  if (BuffInd > 1) {Serial.println("Too long command"); return; } 
  storage.normalizeStorage();
   Serial.println("Storage Normalized. No visible changes exoected.");
}
