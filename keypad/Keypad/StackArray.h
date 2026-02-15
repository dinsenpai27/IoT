#ifndef StackArray_h
#define StackArray_h

#include <Arduino.h>

template<typename T>
class StackArray {
  private:
    struct Node {
      T data;
      Node* next;
    };
    Node* topNode;
  public:
    StackArray() {
      topNode = NULL;
    }

    ~StackArray() {
      while (!isEmpty()) pop();
    }

    void push(const T& item) {
      Node* newNode = new Node;
      newNode->data = item;
      newNode->next = topNode;
      topNode = newNode;
    }

    T pop() {
      if (isEmpty()) return T();
      Node* temp = topNode;
      T item = temp->data;
      topNode = temp->next;
      delete temp;
      return item;
    }

    T peek() {
      if (isEmpty()) return T();
      return topNode->data;
    }

    bool isEmpty() {
      return topNode == NULL;
    }
};

#endif
