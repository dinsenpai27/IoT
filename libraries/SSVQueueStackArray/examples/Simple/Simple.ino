#include "SSVQueueStackArray.h"

// declare a string message.
const String msg = "Happy Hacking!";

// create a storage of characters.
SSVQueueStackArray <char> storage; //default storage parameters: QUEUE_Storage, PTFSA_Resize, 4
//SSVQueueStackArray <char> storage (STACK_Storage); //see constructor for more parameters

// startup point entry (runs once).
void setup () 
{
  // start serial communication.
  Serial.begin (115200);
  Serial.println();
  
  //show storage type
  if (storage.getStorageType() == QUEUE_Storage) 
         Serial.println("Storage Type: QUEUE (FIFO)");
    else Serial.println("Storage Type: STACK (LIFO)");
  
  Serial.println();
  
  // push all the message's characters to the storage.
  for (int i = 0; i < msg.length (); i++)
    storage.push (msg.charAt (i));

  // pop all the message's characters from the stoage.
  while (!storage.isEmpty ())
    Serial.print (storage.pop ());

  // print end of line character.
  Serial.println ();
}

// loop the main sketch.
void loop () 
{
  // nothing here.
}
