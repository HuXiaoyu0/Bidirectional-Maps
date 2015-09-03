#ifndef bidirectional_map_
#define bidirectonal_map_
#include <iostream>
#include <utility>
// -------------------------------------------------------------------
// TREE NODE CLASS 
template <class T1, class T2>
class Node {
public:
  Node() : left(NULL), right(NULL), link(NULL), parent(NULL) {}
  Node(const T1& init) : data(init), left(NULL), right(NULL), link(NULL), parent(NULL) {}
  T1 data;
  Node<T2, T1>* link;
  Node<T1, T2>* left;
  Node<T1, T2>* right;
  Node<T1, T2>* parent; // to allow implementation of iterator increment & decrement
};


// -------------------------------------------------------------------
// TREE NODE ITERATOR CLASS
template <class T1, class T2>
class tree_iterator {
public:
	tree_iterator(): ptr_(NULL), root_(NULL) {}
	tree_iterator(Node<T1,T2>* ptr, Node<T1,T2>* root):ptr_(ptr), root_(root){}
	tree_iterator(const tree_iterator& old) : ptr_(old.ptr_), root_(old.root_){}
	~tree_iterator() {}
	tree_iterator& operator=(const tree_iterator& old) { ptr_ = old.ptr_; root_ = old.root_;return *this; }



	//return to a pointer in this iterator class
	Node<T1, T2>* get_pointer(){return ptr_;}

	
	const std::pair<T1,T2> operator*() const { return std::make_pair(ptr_->data, ptr_->link->data); }

	bool operator==(const tree_iterator& old) const { return (old.ptr_ == this->ptr_ && old.root_==this->root_); }

	bool operator!=(const tree_iterator& old) const { return (old.ptr_ != this->ptr_ || old.root_!=this->root_); }

	//pre-increment
	tree_iterator<T1,T2>&  operator++() { 
    if (ptr_->right != NULL) { // find the leftmost child of the right node
      ptr_ = ptr_->right;
      while (ptr_->left != NULL) { ptr_ = ptr_->left; }
    } else { // go upwards along right branches...  stop after the first left
      while (ptr_->parent != NULL && ptr_->parent->right == ptr_) { ptr_ = ptr_->parent; }
      ptr_ = ptr_->parent;
    }
    return *this;
  }

	//post-increment
	tree_iterator<T1,T2> operator++(int) {
    tree_iterator<T1,T2> temp(*this);
    ++(*this);
    return temp;
  }

	//pre-decrement
	tree_iterator<T1,T2>&  operator--() { 
    if (ptr_ == NULL) { // so that it works for end()
      assert (root_ != NULL);
      ptr_ = root_;
      while (ptr_->right != NULL) { ptr_ = ptr_->right; }
    } else if (ptr_->left != NULL) { // find the rightmost child of the left node
      ptr_ = ptr_->left;
      while (ptr_->right != NULL) { ptr_ = ptr_->right; }
    } else { // go upwards along left brances... stop after the first right
      while (ptr_->parent != NULL && ptr_->parent->left == ptr_) { ptr_ = ptr_->parent; }
      ptr_ = ptr_->parent;
    }
    return *this;
  }

	//post-decrement
	tree_iterator<T1,T2> operator--(int) {
    tree_iterator<T1,T2> temp(*this);
    --(*this);
    return temp;
  }

	//the iterator point to the Node linked with this Node and also the root for the new Node
	const tree_iterator<T2, T1> follow_link() {
	Node<T2,T1>* pointer = this->ptr_->link;
	Node<T2,T1>* ptr = pointer;
	while(pointer->parent != NULL)
		pointer = pointer->parent;
	tree_iterator<T2,T1> temp (ptr,pointer);
	return temp;
	}








private:
	Node<T1, T2>* ptr_; //pointer to a Node
	Node<T1, T2>* root_; //pointer to the same node that the root pointing to
};


// -------------------------------------------------------------------
// bidirectional_map class
template <class T1, class T2>
class bidirectional_map {
public:
	typedef tree_iterator<T1,T2> key_iterator;
	typedef tree_iterator<T2,T1> value_iterator;
	//friend class tree_iterator<T1, T2>;

	bidirectional_map() : key_root_(NULL), value_root_(NULL), size_(0) {}

	~bidirectional_map() {
		destroy_key(key_root_); key_root_ = NULL;
		destroy_value(value_root_); value_root_ = NULL;
	}
	
	//copy constructor
	//call functions copy_key and copy_value to copy the two trees
	//then  rebuild the links
	bidirectional_map(const bidirectional_map<T1,T2>& old) : size_(old.size_) {
		key_root_ = this->copy_key(old.key_root_,NULL);
		value_root_ = this->copy_value(old.value_root_,NULL);
		key_iterator itr = old.key_begin();

		for (key_iterator ite = this->key_begin(); ite != this->key_end(); ite++){
			ite.get_pointer()->link = (find_value(itr.get_pointer()->link->data, value_root_)).get_pointer();
			ite.get_pointer()->link->link = ite.get_pointer();
			itr++;
		}
	}

	//destroy the old class values and point to the assigned map
	bidirectional_map& operator=(const bidirectional_map<T1,T2>& old) {
		if (&old != this) {
			this->destroy_key(key_root_);
			this->destroy_value(value_root_);
			key_root_ = this->copy_key(old.key_root_, NULL);
			value_root_ = this->copy_value(old.value_root_, NULL);
			size_ = old.size_;
			key_iterator itr = old.key_begin();
			for (key_iterator ite = this->key_begin(); ite != this->key_end(); ite++){
			value_iterator g = this->find_value(itr.ptr_->link->data, value_root_);
			ite.get_pointer()->link = g.get_pointer();
			g.get_pointer()->link = ite.get_pointer();
			itr++;
		}
		}
		return *this;
	}


	//call find function and return the value or key
	T2 operator[] (T1 key) {return this->find(key).get_pointer()->link->data;}
	T1 operator[] (T2 value) {return this->find(value).get_pointer()->link->data;} 
	int size() const { return size_; }


	//--------------------------------------------------------------------------------
	//call insert_value and insert_key to extend the old trees and link them together
	std::pair<key_iterator, bool > insert(const std::pair<T1, T2> pair){
		Node<T1,T2>* NULL_pointer_key(NULL);
		Node<T2,T1>* NULL_pointer_value(NULL);
		std::pair<tree_iterator<T1,T2>, bool> insert_result_key = insert_key(pair.first, this->key_root_, NULL_pointer_key);
		if (insert_result_key.second){
			std::pair<tree_iterator<T2,T1>, bool> insert_result_value = insert_value(pair.second, this->value_root_, NULL_pointer_value);
			insert_result_value.first.get_pointer()->link = insert_result_key.first.get_pointer();
			insert_result_key.first.get_pointer()->link = insert_result_value.first.get_pointer();
			return insert_result_key;
		}
		else
			return insert_result_key;
	}
	//--------------------------------------------------------------------------------
	int erase(T1 const& key) {
		key_iterator ite = this->find_key(key,key_root_);
		if (ite == this->key_end())
			return 0;
		else{
		T2 value = ite.get_pointer()->link->data;
	    erase_value(value,value_root_);
		//this->print(std::cout);
		return erase_key(key, key_root_); }}


	//print like sideway
	void print(std::ostream& ostr){
	ostr<<"================================================="<<std::endl;
	ostr<<"KEYS:"<<std::endl;
	print_key(ostr, key_root_, 0);
	ostr<<"-------------------------------------------------"<<std::endl;
	ostr<<"VALUES:"<<std::endl;
	print_value(ostr, value_root_, 0);
	ostr<<"================================================="<<std::endl;
	}

	//return to a iterator pointing to a node that holds the key or value
	tree_iterator<T1,T2> find(T1 key){
	return find_key(key, key_root_);
	}
	tree_iterator<T2,T1> find(T2 value){
	return find_value(value, value_root_);
	}

	//return a NULL Node
	key_iterator key_end() const { return tree_iterator<T1, T2>(NULL,this->key_root_); }
	value_iterator value_end() const { return tree_iterator<T2, T1>(NULL,this->value_root_); }

	//return the node that the root pointer pointing to
	key_iterator key_begin() const{ 
    if (!key_root_) return tree_iterator<T1,T2>(NULL,NULL);
    Node<T1,T2>* p = key_root_;
    while (p->left) p = p->left;
	return tree_iterator<T1, T2>(p,this->key_root_);
  }
	value_iterator value_begin() const{
    if (!value_root_) return tree_iterator<T2,T1>(NULL,NULL);
    Node<T2,T1>* p = value_root_;
    while (p->left) p = p->left;
	return tree_iterator<T2, T1>(p,this->value_root_);
  }

	
private:
	Node<T1, T2>* key_root_;
	Node<T2, T1>* value_root_;
	int size_;

	//destroy the key tree and free the memory
	void destroy_key(Node<T1,T2>* p) {
		if (!p) return;
		destroy_key(p->left);
		destroy_key(p->right);
		delete p;
	}

	//destroy the value tree and free the memory
	void destroy_value(Node<T2,T1>* p) {
		if (!p) return;
		destroy_value(p->left);
		destroy_value(p->right);
		delete p;
	}

	//copy the left tree
	Node<T1,T2>* copy_key(Node<T1,T2>* key_root, Node<T1,T2>* the_parent){
	if (key_root == NULL) return NULL;
	Node<T1,T2> *answer = new Node<T1,T2>();
	answer->data = key_root->data;
	answer->left = copy_key(key_root->left, answer);
	answer->right = copy_key(key_root->right, answer);
	answer->parent = the_parent;
	return answer;
	}

	//copy the right tree
	Node<T2,T1>* copy_value(Node<T2,T1>* value_root, Node<T2,T1>* the_parent){
	if (value_root == NULL) return NULL;
	Node<T2,T1> *answer = new Node<T2,T1>();
	answer->data = value_root->data;
	answer->left = copy_value(value_root->left, answer);
	answer->right = copy_value(value_root->right, answer);
	answer->parent = the_parent;
	return answer;
	}

	//return the pointer to the newly insert value and a boolean value indicate whether the insert is successful or not
	std::pair<key_iterator,bool> insert_key(const T1& data_key, Node<T1, T2>*& p_key, Node<T1, T2>* parent){
		
	if (!p_key) {
      p_key = new Node<T1, T2> (data_key);
      p_key->parent = parent;
      this->size_++;
	  return std::pair<tree_iterator<T1,T2>,bool>(tree_iterator<T1,T2> (p_key,this->key_root_), true);
    }
	else if (data_key < p_key->data)
      return insert_key(data_key, p_key->left, p_key);
	else if (data_key > p_key->data)
      return insert_key(data_key, p_key->right, p_key);
    else
		return std::pair<tree_iterator<T1,T2>,bool>(tree_iterator<T1,T2> (p_key,this->key_root_), false);
}

	//return the pointer to the newly insert value and a boolean value indicate whether the insert is successful or not
	std::pair<tree_iterator<T2,T1>,bool> insert_value(const T2& data_value, Node<T2, T1>*& p_value, Node<T2, T1>* parent){
		
	if (!p_value) {
      p_value = new Node<T2, T1> (data_value);
      p_value->parent = parent;
      
	  return std::pair<tree_iterator<T2,T1>,bool>(tree_iterator<T2,T1> (p_value,this->value_root_), true);
    }
	else if (data_value < p_value->data)
      return insert_value(data_value, p_value->left, p_value);
	else if (data_value > p_value->data)
		return insert_value(data_value, p_value->right, p_value);
    else
		return std::pair<tree_iterator<T2,T1>,bool>(tree_iterator<T2,T1> (p_value,this->value_root_), false);
	}

	//erase the Node and rebuild part of the tree if necessary
	int erase_key(T1 const& key, Node<T1,T2>* &p) {
    if (!p) return 0;
 
    // look left & right
    if (p->data < key)
      return erase_key(key, p->right);
    else if (p->data > key)
      return erase_key(key, p->left);

    // Found the node.  Let's delete it
    assert (p->data == key);
    if (!p->left && !p->right) { // leaf
      delete p; 
      p=NULL;       
      this->size_--;
    } else if (!p->left) { // no left child
      Node<T1,T2>* q = p; 
      p=p->right; 
      assert (p->parent == q);
      p->parent = q->parent;
      delete q; 
      this->size_--;    
    } else if (!p->right) { // no right child
      Node<T1,T2>* q = p; 
      p=p->left;
      assert (p->parent == q);
      p->parent = q->parent;
      delete q; 
      this->size_--;
    } else { // Find rightmost node in left subtree
      Node<T1,T2>* q = p->left;
      while (q->right) q = q->right;
	  p->data = q->data;
	  p->link = q->link;
	  p->link->link = p;
      // recursively remove the value from the left subtree
      int check = erase_key(q->data, p->left);
      assert (check == 1);
    }
    return 1;
  }

	//erase the Node and rebuild part of the tree if necessary 
	int erase_value(T2 const& value, Node<T2,T1>* &p) {
    if (!p) return 0;
 
    // look left & right
   if (p->data < value)
      return erase_value(value, p->right);
    else if (p->data > value)
      return erase_value(value, p->left);

    // Found the node.  Let's delete it
    assert (p->data == value);
    if (!p->left && !p->right) { // leaf
      delete p; 
      p=NULL;       
      //this->size_--;    
    } else if (!p->left) { // no left child
      Node<T2,T1>* q = p; 
      p=p->right; 
      assert (p->parent == q);
      p->parent = q->parent;
      delete q; 
      //this->size_--;    
    } else if (!p->right) { // no right child
      Node<T2,T1>* q = p; 
      p=p->left;
      assert (p->parent == q);
      p->parent = q->parent;
      delete q; 
      //this->size_--;
    } else { // Find rightmost node in left subtree
      Node<T2,T1>* q = p->left;
      while (q->right) q = q->right;
	  p->data = q->data;
	  p->link = q->link;
	  p->link->link = p;
	  //this->print(std::cout);
      // recursively remove the value from the left subtree
      int check = erase_value(q->data, p->left);
      assert (check == 1);
    }
    return 1;
  }




	void print_key(std::ostream& ostr, Node<T1,T2>* key_root, int depth_key){
	if (key_root) {
      print_key(ostr, key_root->right, depth_key+1);
      for (int i=0; i<depth_key; ++i) ostr << "    ";
	  ostr << key_root->data <<" ["<<key_root->link->data<<"]"<< "\n";
      print_key(ostr, key_root->left, depth_key+1);
    }
	}
	
	void print_value(std::ostream& ostr, Node<T2,T1>* value_root, int depth_value){
	if (value_root) {
      print_value(ostr, value_root->right, depth_value+1);
      for (int i=0; i<depth_value; ++i) ostr << "    ";
      ostr << value_root->data <<" ["<<value_root->link->data<<"]"<< "\n";
      print_value(ostr, value_root->left, depth_value+1);
    }
	}
	
	//return a key_iterator pointing to the node that holds that key
	tree_iterator<T1,T2> find_key(const T1& key, Node<T1,T2>* p) {
    if (!p) return key_end();
    if (p->data > key)
      return find_key(key, p->left);
    else if (p->data < key)
      return find_key(key, p->right);
    else
	  return tree_iterator<T1,T2>(p,this->key_root_);
  }

	//return a value iterator pointing to the node taht holds that value
	tree_iterator<T2,T1> find_value(const T2& value, Node<T2,T1>* p) {
    if (!p) return value_end();
    if (p->data > value)
      return find_value(value, p->left);
    else if (p->data < value)
      return find_value(value, p->right);
    else
	  return tree_iterator<T2,T1>(p,this->value_root_);
  }

};

#endif
