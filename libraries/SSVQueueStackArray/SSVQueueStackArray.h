/*
SSVQueueStackArray.h
Author Serge Skorodinsky 
1/24/2021
Inspired by library from https://codebender.cc/example/QueueArray/ManageString#ManageString.ino
                and from https://playground.arduino.cc/Code/QueueArray/
				
Home: https://github.com/SergeSkor/SSVQueueStackArray				

*/

#ifndef _SSVQUEUESTACKARRAY_H
#define _SSVQUEUESTACKARRAY_H

//#include <Arduino.h> // include Arduino basic header - not really needed (by SSV)  

#ifndef ARDUINO
#include <stdlib.h> //for CCS or other compilers, not needed for Arduino
#endif

//uncomment to see debug messages
//#define DEBUG_QUEUE_STACK_ARRAY

//    Push-To-Full-Storage-Action (PTFSAction) type:
//      PTFSA_Resize:    resize (up and down) is allowed;
//      PTFSA_Overwrite: resize is not allowed, new item will overwrite the oldest one;
//      PTFSA_Ignore:    resize is not allowed, new item will be ignored.
enum PTFSAction {PTFSA_Resize=0, PTFSA_Overwrite=1, PTFSA_Ignore=2};

//  StorageType type: Queue (FIFO), Stack (LIFO)
enum StorageType {QUEUE_Storage=0, STACK_Storage=1}; 

// the definition of the QueueStackArray class.
template<typename T> class SSVQueueStackArray
{
  //limit max size??? No.
  public:
    SSVQueueStackArray(const StorageType atype=QUEUE_Storage, const PTFSAction action=PTFSA_Resize, const unsigned int initSize = 4 ); //default constructor (with parameters!)
    ~SSVQueueStackArray();    //destructor.
    void push (const T item); //push an item to the Storage.
    T pop();  //pop an item from the Storage, oldest if type is QUEUE_Storage, newest if type is STACK_Storage
    bool isEmpty() const;     //check if the Storage is empty.
    unsigned int getCount() const;    //get the number of items in the Storage.
    bool isFull() const;  //check if the Storage is full.
    unsigned int getSize() const;  //get the Storage Size (Capacity).
    T getDataByIndex(const unsigned int index) const;  // get an item from the Storage by index, 0 is always OLDEST. No changes in Storage!
    void setPTFSAction(const PTFSAction action);  //Set the Push-T0-Full-Storage-Action: PTFSA_Resize, PTFSA_Overwrite, PTFSA_Ignore
    PTFSAction getPTFSAction() const;             //Get the Push-T0-Full-Storage-Action: PTFSA_Resize, PTFSA_Overwrite, PTFSA_Ignore
    void setStorageType(const StorageType atype); //Set StorageType: QUEUE_Storage (FIFO) or STACK_Storage (LIFO)
    StorageType getStorageType() const;           //Get StorageType: QUEUE_Storage (FIFO) or STACK_Storage (LIFO)
    //moved from private to public
    void resize(const unsigned int newSize); // resize the size of the Storage.
    //added 1/24/2021
    void resetData();  //reset the Storage, flush all data. All other properties are not changing. NO resize!
    void normalizeStorage(); //normalize Storage by re-allocating memory. Logical index is now matching to the real one. Actually, resize to the same size. 
    //convenience functions. added 1/30/2021
    T peekNewest() const;  //returns last (the newest) pushed item. No changes in Storage! The same as getDataByIndex(Count-1).
    T peekOldest() const;  //returns first (the oldest) pushed item. No changes in Storage! The same as getDataByIndex(0).
    T popNewest();  //pop an item from the Storage, the same as pop() when STACK_Storage.
    T popOldest();  //pop an item from the Storage, the same as pop() when QUEUE_Storage.
    
  private:
    unsigned int getRealIndex(const unsigned int index) const; //convert logical index (where [0] is the oldest) to real index.
    T * fDataArr;    //the main Storage.
    unsigned int fSize;   //the size of the Storage.
    unsigned int fCount;  //the number of items of the Storage.
    unsigned int fFirstAvailInd; //pointing to first available index, will increments each push.
    PTFSAction fPTFSAction; //Push-T0-Full-Storage-Action: PTFSA_Resize, PTFSA_Overwrite, PTFSA_Ignore
    StorageType fStorageType;  //QUEUE_Storage (FIFO) or STACK_Storage (LIFO)

};

//default constructor (with parameters!)
template<typename T> SSVQueueStackArray<T>::SSVQueueStackArray(const StorageType atype, const PTFSAction action, const unsigned int initSize)
{
  unsigned int realInitSize;
  if (initSize == 0) realInitSize = 1; else realInitSize = initSize;  //initSize cannot be zero, replace with 1.
  fSize = 0;
  fCount = 0;
  fDataArr = (T *) malloc (sizeof (T) * realInitSize);  // allocate enough memory for the Storage.
  if (fDataArr == NULL)
    {
	  #ifdef DEBUG_QUEUE_STACK_ARRAY
	  #ifdef ARDUINO
    Serial.printf("QueueStackArray constructor error: insufficient memory to initialize queue.\r\n");  // if there is a memory allocation error.
    #endif
	  #endif
	  return;
    }
  fSize = realInitSize;     // set the initial size of the QueueStackArray.
  fFirstAvailInd = 0;
  fPTFSAction = action; //PTFQA_Resize=0, PTFQA_Overwrite=1, PTFQA_Ignore=2
  fStorageType = atype; //QUEUE_Storage (FIFO), STACK_Storage (LIFO)
}

// destructor.
template<typename T> SSVQueueStackArray<T>::~SSVQueueStackArray()
{
  free(fDataArr); // deallocate the array of the QueueStackArray.
  fDataArr = NULL; // set QueueStackArray array pointer to nowhere.
}

// resize the size of the QueueStackArray.
template<typename T> void SSVQueueStackArray<T>::resize(const unsigned int newSize)
{
  unsigned int realNewSize;
  if (newSize == 0) realNewSize = 1; else realNewSize = newSize;  //newSize cannot be zero, replace with 1.
  #ifdef DEBUG_QUEUE_STACK_ARRAY
  #ifdef ARDUINO
  Serial.printf("QueueStackArray resize: from %d to %d.\r\n", fSize, realNewSize);
  #endif
  #endif
  if (realNewSize < fCount) //(was: "<=". Changed to make normalize() working)
    {
	  #ifdef DEBUG_QUEUE_STACK_ARRAY
	  #ifdef ARDUINO
    Serial.printf("QueueStackArray resize error: new size (%d) must be not less than Count(%d).\r\n", realNewSize, fCount);
	  #endif
	  #endif
    return;
    }
  T * temp = (T *) malloc (sizeof (T) * realNewSize);    // allocate enough memory for the temporary array.
  if (temp == NULL) 
    {
	  #ifdef DEBUG_QUEUE_STACK_ARRAY
	  #ifdef ARDUINO
    Serial.printf("QueueStackArray resize error: insufficient memory to initialize temporary queue.\r\n"); // if there is a memory allocation error.
	  #endif
	  #endif
    return;
    }

  for (unsigned int i = 0; i < fCount; i++)  temp[i] = fDataArr[getRealIndex(i)]; // copy the items from the old queue to the new one.
  free(fDataArr);   // deallocate the old array of the QueueStackArray.
  fDataArr = temp; // copy the pointer of the new QueueStackArray.
  fSize = realNewSize;   // set the new size of the QueueStackArray.
  fFirstAvailInd = fCount;
}

// push an item to the Storage.
template<typename T> void SSVQueueStackArray<T>::push(const T item)
{
  if (isFull()) //check if the QueueStackArray is full?
    {
      switch (fPTFSAction)
      {
      case (PTFSA_Resize):
        {
		    #ifdef DEBUG_QUEUE_STACK_ARRAY
			  #ifdef ARDUINO
        Serial.printf("QueueStackArray push: QueueStackArray is full, RESIZE requested.\r\n");
		    #endif
			  #endif
        resize(fSize * 2); //double size of array.
        //regular insert now
        fDataArr[fFirstAvailInd] = item;
        fFirstAvailInd++;
        fCount++;
        return;
        }
      case (PTFSA_Overwrite):
        {
        //resize is not allowed, but overwrite is allowed
        fFirstAvailInd = fFirstAvailInd % fSize;
        fDataArr[fFirstAvailInd] = item;
        fFirstAvailInd++;
		    #ifdef DEBUG_QUEUE_STACK_ARRAY
			  #ifdef ARDUINO
        Serial.printf("QueueStackArray push: QueueStackArray is full, OVERWRITE. \r\n");
		    #endif
			  #endif
        return;
        }
      default: //case (PTFQA_Ignore):
        {
        //resize is not allowed, overwrite is not allowed -> no changes, just terminate
		    #ifdef DEBUG_QUEUE_STACK_ARRAY
			  #ifdef ARDUINO
        Serial.printf("QueueStackArray push: QueueStackArray is full, IGNORED. \r\n");
		    #endif
			#endif
        return;
        }
      }
    }
  else
    {
    //not full, regular insert and increment index
    fFirstAvailInd = fFirstAvailInd % fSize;
    fDataArr[fFirstAvailInd] = item;
    fFirstAvailInd++;
    fCount++;
    return;
    }
}

// pop an item from the Storage, oldest if type is QUEUE_Storage, newest if type is STACK_Storage.
template<typename T> T SSVQueueStackArray<T>::pop()
{

  if (fStorageType == STACK_Storage) 
    return popNewest();
  else 
    return popOldest();
  
  /*   
  replaced with popOldest() or popNewest()
  if (isEmpty()) 
    {
	  #ifdef DEBUG_QUEUE_STACK_ARRAY
	  #ifdef ARDUINO
    Serial.printf("QueueStackArray pop error: can't pop item from QueueStackArray, QueueStackArray is empty.\r\n");  // check if the queue is empty.
	  #endif
	  #endif
    return T(); //default constructor for T  
    }

  T item;
  if (fStorageType == STACK_Storage)
    {
    //STACK
    item = fDataArr[getRealIndex(fCount-1)]; // fetch the newest item from the Storage.
    if (fFirstAvailInd==0) fFirstAvailInd = fSize-1; else fFirstAvailInd--;
    }
    else
      {
      //QUEUE
      item = fDataArr[getRealIndex(0)]; // fetch the oldest item from the Storage.
      }

  fCount--;
  if ( (!isEmpty()) && (fCount <= fSize/4) && (fPTFSAction == PTFSA_Resize) ) resize(fSize / 2); // shrink size of array if necessary.
  return item;
  */
}

// get an item from the Storage by logical index, 0 - OLDEST!
template<typename T> T SSVQueueStackArray<T>::getDataByIndex(const unsigned int index) const
{
  if (isEmpty()) 
    {
	  #ifdef DEBUG_QUEUE_STACK_ARRAY
	  #ifdef ARDUINO
    Serial.printf("QueueStackArray getDataByIndex error: can't peek item from QueueStackArray, QueueStackArray is empty.\r\n");
	  #endif
	  #endif
    return T(); //default constructor for T  
    }
  if (index >= fCount) 
    {
	  #ifdef DEBUG_QUEUE_STACK_ARRAY
	  #ifdef ARDUINO
    Serial.printf("QueueStackArray getDataByIndex error: can't peek item from QueueStackArray: index (%d) must be lower than Count(%d)\r\n", index, fCount);
	  #endif
	  #endif
    return T(); //default constructor for T  
    }

  unsigned int real_ind = getRealIndex(index);   //
  return fDataArr[real_ind];
}

//convert logical index (where [0] is the oldest) to real index.
template<typename T> unsigned int SSVQueueStackArray<T>::getRealIndex(const unsigned int index) const
{
return (fFirstAvailInd  - fCount + index) % fSize; 
}

// check if the Storage is empty.
template<typename T> bool SSVQueueStackArray<T>::isEmpty() const
{
  return (fCount == 0);
}

// check if the Storage is full.
template<typename T> bool SSVQueueStackArray<T>::isFull() const
{
  return (fCount == fSize);
}

// get the number of items in the Storage.
template<typename T> unsigned int SSVQueueStackArray<T>::getCount() const
{
  return fCount;
}

// get the Capacity
template<typename T> unsigned int SSVQueueStackArray<T>::getSize() const
{
  return fSize;
}

//Get the Push-T0-Full-Storage-Action: PTFSA_Resize, PTFSA_Overwrite, PTFSA_Ignore
template<typename T> PTFSAction SSVQueueStackArray<T>::getPTFSAction() const
{
  return fPTFSAction;
}

//Set the Push-T0-Full-Storage-Action: PTFSA_Resize, PTFSA_Overwrite, PTFSA_Ignore
template<typename T> void SSVQueueStackArray<T>::setPTFSAction(const PTFSAction action)
{
  fPTFSAction = action;
}

template<typename T> StorageType SSVQueueStackArray<T>::getStorageType() const
{
  return fStorageType;
}

template<typename T> void SSVQueueStackArray<T>::setStorageType(const StorageType atype)
{
  fStorageType = atype;
}

//reset the Storage, flush all data. All other properties are not changing. No resize!
template<typename T> void SSVQueueStackArray<T>::resetData()
{
fCount=0;
fFirstAvailInd=0;
}

//normalize Storage by re-allocating memory. The same as resize to the same size.
template<typename T> void SSVQueueStackArray<T>::normalizeStorage()
{
resize(fSize);
}


//returns last (the newest) pushed item. No changes in Storage! The same as getDataByIndex(Count-1).
template<typename T> T SSVQueueStackArray<T>::peekNewest() const
{
  return getDataByIndex(fCount-1);
}

//returns first (the oldest) pushed item. No changes in Storage! The same as getDataByIndex(0).
template<typename T> T SSVQueueStackArray<T>::peekOldest() const
{
  return getDataByIndex(0);
}

//pop an item from the Storage, the same as pop() when STACK_Storage.
template<typename T> T SSVQueueStackArray<T>::popNewest()
{
  if (isEmpty()) 
    {
    #ifdef DEBUG_QUEUE_STACK_ARRAY
    #ifdef ARDUINO
    Serial.printf("QueueStackArray popNewest error: can't pop item from QueueStackArray, QueueStackArray is empty.\r\n");  // check if the queue is empty.
    #endif
    #endif
    return T(); //default constructor for T  
    }

  T item;
  //as for STACK
  item = fDataArr[getRealIndex(fCount-1)]; // fetch the newest item from the Storage.
  if (fFirstAvailInd==0) fFirstAvailInd = fSize-1; else fFirstAvailInd--;
  //
  fCount--;
  if ( (!isEmpty()) && (fCount <= fSize/4) && (fPTFSAction == PTFSA_Resize) ) resize(fSize / 2); // shrink size of array if necessary.
  return item;
}

//pop an item from the Storage, the same as pop() when QUEUE_Storage.
template<typename T> T SSVQueueStackArray<T>::popOldest()
{
  if (isEmpty()) 
    {
    #ifdef DEBUG_QUEUE_STACK_ARRAY
    #ifdef ARDUINO
    Serial.printf("QueueStackArray popOldest error: can't pop item from QueueStackArray, QueueStackArray is empty.\r\n");  // check if the queue is empty.
    #endif
    #endif
    return T(); //default constructor for T  
    }

  T item;
  //as for QUEUE
  item = fDataArr[getRealIndex(0)]; // fetch the oldest item from the Storage.
  //
  fCount--;
  if ( (!isEmpty()) && (fCount <= fSize/4) && (fPTFSAction == PTFSA_Resize) ) resize(fSize / 2); // shrink size of array if necessary.
  return item;
}

#endif // _SSVQUEUESTACKARRAY_H
