#include <string.h>
#include <cassert>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
using namespace std;

#define SIZE 50
int hashCode(std::string const & key)

{
  const long x = 41L;
  int n = key.length();
  long value = 0L;
  while( n-- > 0 )
  {
    value *= x;   value += key[n];
  }
  return static_cast<int>(value);
}

struct file
{
  string entryNo;
  string name;
  string balance;
};

std::ostream& operator << (std::ostream& out, file& c) {
  out << c.entryNo << " : " << c.name<<":";
  return out;
}
struct Node
{
  Node *next;
};

struct Pair {
  int first;
  file  *second;

  ~Pair() {
    if(second != NULL)
      delete second;
  }
};

struct InternalNode : public Node
{
  Pair *element;

  ~InternalNode() {
    delete element;
  }
};

class LinkedList
{
public:
  LinkedList() {
    head = new Node();
    head->next = NULL;
  }
  ~LinkedList() {
    del(head);
  }
  void insert(file & elem, int hashkey) {
    Pair *data = new Pair();
    data->first = hashkey;
    data->second = &elem;
    InternalNode *temp = new InternalNode();
    temp->next = head->next;
    temp->element = data;
    head->next = temp;
  }
  file* search(int hashkey)
  {
    Node *current = head->next;
    while(current != NULL)
    {
      if(static_cast<InternalNode*>(current)->element->first == hashkey)
        return static_cast<InternalNode*>(current)->element->second;
      current = current->next;
    }
    return NULL;
  }
  void remove(int hashkey)
  {
    Node *current = head->next;
    Node *parent = head;
    while(current != NULL)
    {
      if(static_cast<InternalNode*>(current)->element->first == hashkey)
      {
        parent->next = current->next;
        current->next = NULL;
        del(current);
        return;
      }
      parent = current;
      current = current->next;
    }
  }
  void removeAll()
  {
    del(head->next);
    head->next = NULL;
  }
  void print()
  {
    InternalNode *current = static_cast<InternalNode*>(head->next);
    int i = 1;
    while(current != NULL)
    {
      std::cout << i << " : hashkey " <<std::hex << static_cast<unsigned int>( current->element->first ) << ") <" << *(current->element->second) << ">" << std::endl;
      current = static_cast<InternalNode*>(current->next);
      ++i;
    }
  }
  Node *getList()
  {
    return head->next;
  }
private:
  void del(Node *node)
  {
    if(node == NULL)
      return;

    del(node->next);
    delete node;
  }
  Node *head;
};

class Bucket
{
public:
  Bucket(int c, int s = 0) : sig_bits(s), cap(c), sz(0), code(0), root(false)
  {
    data = new LinkedList();
  }
  ~Bucket()
  {
    delete data;
  }
  int capacity()
  {
    return cap;
  }
  int size()
  {
    return sz;
  }
  void insert(file& elem, int hashkey)
  {
    data->insert(elem, hashkey);
    ++sz;
    if(root)
      return;
    assert( ( size() <= capacity() ) );
  }
  file* search(int hashkey)
  {
    return data->search(hashkey);
  }
  void remove(int hashkey)
  {
    if(search(hashkey) != NULL)
      --sz;
    data->remove(hashkey);
    assert( (size() >= 0) );
  }
  void emptyBucket()
  {
    data->removeAll();
    sig_bits = 0;
    sz = 0;
  }
  bool isEmpty()
  {
    return size() == 0;
  }
  void print()
  {
    cout << "BUCKET";
    printcode();
    cout << "" <<endl;
    data->print();
    cout << endl;
  }
  Node* getList()
  {
    return data->getList();
  }
  int getSIG_BITS()
  {
    return sig_bits;
  }
  void setSIG_BITS(int s)
  {
    sig_bits = s;
    assert( (sig_bits >= 0) );
  }
  void setCODE(int c)
  {
    code = c;
  }
  void setROOT(bool state)
  {
    root = state;
  }
private:
  void printcode()
  {
    int length = sig_bits;
    int temp = code;
    while(length-- > 0)
    {
      std::cout << temp%2;
      temp >>= 1;
    }
  }
  LinkedList *data;
  int sig_bits;
  int cap;
  int sz;
  int code;
  bool root;
};

struct TrieNode
{
  Bucket *page;
  TrieNode *next[2];
  TrieNode *parent;

  ~TrieNode()
  {
    if(page != NULL)
      delete page;
  }
};

class HashTable
{
public:
  HashTable(int buk = DFL_BUCKET_SIZE) : bucket_size(buk)
  {
    sig_bits = 0;
    index = new TrieNode();
    index->page = new Bucket(bucket_size, sig_bits);
    index->next[0] = NULL;
    index->next[1] = NULL;
    index->parent = NULL;
    no_of_buckets = 1;
  }
  ~HashTable()
  {
    del(index);
  }
  file* search(std::string number)
  {
    int code;
    int hashkey = hashCode(number);
    TrieNode *node = findNode(hashkey, &code);
    return node->page->search(hashkey);
  }
  void insert(file ck)
  {

    file & cb = *new file();
    cb.name = ck.name;
    cb.entryNo = ck.entryNo;
    cb.balance=ck.balance;
    int hashkey = hashCode(cb.entryNo);
    int code;
    TrieNode *node = findNode(hashkey, &code);
    Bucket *page = node->page;
    if(page->size()+1 <= page->capacity())
    {
      page->insert(cb, hashkey);
      return;
    } else
    {
      Bucket *newPage = new Bucket(bucket_size);
      no_of_buckets++;
      LinkedList *Q = new LinkedList();
      InternalNode *current = static_cast<InternalNode*>( page->getList() );
      while(current != NULL)
      {
        Q->insert(*(current->element->second), current->element->first);
        current = static_cast<InternalNode*>(current->next);
      }
      Q->insert(cb, hashkey);
      int oldD = page->getSIG_BITS();
      int MASK = 1 << oldD;
      newPage->setSIG_BITS(oldD + 1);
      page->emptyBucket();
      page->setSIG_BITS(oldD + 1);
      if(oldD+1 > sig_bits)
      {
        ++sig_bits;
      }
        node->page = NULL;
        TrieNode *left = new TrieNode();
        TrieNode *right = new TrieNode();
        node->next[0] = left;
        node->next[1] = right;
        left->page = page;
        right->page = newPage;
        left->next[0] = left->next[1] = NULL; left->parent = node;
        right->next[0] = right->next[1] = NULL; right->parent = node;
        left->page->setCODE(code);
        right->page->setCODE(code | MASK);

      current = static_cast<InternalNode*>( Q->getList() );
      while(current != NULL)
      {
        insert( *(current->element->second) );
        current = static_cast<InternalNode*>( current->next );
      }
    }
  }
  void remove(std::string number)
  {
    int hashkey = hashCode(number);
    int code;
    TrieNode *node = findNode(hashkey, &code);
    Bucket *page = node->page;
    if(page->search(hashkey) == NULL)
      return;
    page->remove(hashkey);
    if(page->size() == 0 && node->parent != NULL)
    {
      TrieNode *parent = node->parent;
      TrieNode *sibling = (parent->next[0] == node)?
            (parent->next[1]) : (parent->next[0]);
      if(sibling->next[0] != NULL || sibling->next[1] != NULL)
        return;

      no_of_buckets--;
      parent->page = sibling->page;
      sibling->page = NULL;
      parent->next[0] = NULL;
      parent->next[1] = NULL;
      del(node);
      del(sibling);
      parent->page->setSIG_BITS(parent->page->getSIG_BITS() - 1);
      int MASK = 1 << parent->page->getSIG_BITS();
      MASK -= 1;
      code &= MASK;
      parent->page->setCODE(code);
      sig_bits = max_bits(index);
    }
  }
  void print()
  {
    char data[100];
    cout << "\n" << std::endl;
    cout << "bucket.capacity : " << bucket_size << "\t \t";
    cout << "number ( buckets ) : " << no_of_buckets << "\t \t";
    int total = printNode(index);
    cout << "elements in hashtable : " << total << std::endl;
    };
   
private:
  TrieNode* findNode(int hashkey, int *const code)
  {
    int d = 1;
    d <<= sig_bits;
    d -= 1;
    int key = hashkey & d;
    *code = 0;
    int length = 0;
    TrieNode *current = index;
    while(current != NULL) {
      if(current->page != NULL)
        return current;
      if(current->next[key & 1] != NULL)
      {
        current = current->next[key & 1];
        int MASK = key&1;
        if(length != 0)
          MASK <<= length;
        *code |= MASK;
        key >>= 1;
        ++length;
      }
    }
    assert( 0 );
    return NULL;
  }
  void del(TrieNode *node)
  {
    if(node == NULL)
      return;
    TrieNode *left, *right;
    left = node->next[0];
    right = node->next[1];
    del(left);
    del(right);
    delete node;
  }
  int printNode(TrieNode *current)
  {
    if(current == NULL)
      return 0;
    if(current->page != NULL)
    {
      current->page->print();
      return current->page->size();
    }
    int number = printNode(current->next[0]);
    number += printNode(current->next[1]);
    return number;
  }
  int max_bits(TrieNode *node)
  {
    if(node->page != NULL)
      return node->page->getSIG_BITS();
    int left = max_bits(node->next[0]);
    int right = max_bits(node->next[1]);
    int max = (left > right) ? left : right;
    return max;
  }
  const static int DFL_BUCKET_SIZE = 4;
  TrieNode *index;
  int sig_bits;
  int bucket_size;
  int no_of_buckets;
};

int printMenu()
{
  cout << "Extendible Hashing" << endl;
  cout << "\t\t 1. Insert a Record" << endl;
  cout << "\t\t 2. Search a Record" << endl;
  cout << "\t\t 3. Show All Records"<< endl;
  cout << "\t\t 4. Exit" << endl;
  int choice;
  cout << "\ninsert your choice : ";
  cin >> choice;
  cout.flush();
  return choice;
}


int main()
{
  int n=1;
    char ch;
  HashTable *table = new HashTable(n);
  while(true)
    {
    int choice = printMenu();
    ch = getchar();
    switch(choice) {
      case 1:
      {
        char buf[SIZE];
        file cb;
        cout << "\nEnter description : ";
        getline(std::cin, cb.name);
        cout << "Enter Entry Number : ";
        getline(std::cin, cb.entryNo);
        fstream ofile;
        ofile.open("file.txt",ios::app);
        ofile<<cb.name<<"\t"<<cb.entryNo<<"\t";
        table->insert(cb);
        ofile.close();
        break;
      }
      case 2:
      {
        string entNum;
        cout << "\nEnter Entry Number: ";
        getline(std::cin, entNum);
        file *cb = table->search(entNum);
        cout << *cb;
        cout << std::endl <<"enter to continue ...";
        char ch = std::getchar();
        break;
      }
      
      case 3:

        table->print();
        cout << endl << "enter to continue ...";
        ch = getchar();
        break;
      case 4:
        goto DONE;
        }
    }
  DONE:
  delete table;
  return 0;
}

