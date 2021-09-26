#pragma once
#include <iterator>
#include <stack>
#include<iostream>
#include <string>
#include <exception>
#include <algorithm>
#include <vector>
#include <ctime>
using namespace std;
const int lim = 100; //������������ ��� ��������� ������

struct Node {	//���� ���-������
	int key;	//���
	Node* link[2];	//����� � ������ ���������
	int b;       //������: -1, 0, +1
	Node(int k) : key(k), b(0) { link[0] = link[1] = nullptr; }
	//	Node(int k, Node* d = nullptr) : key(k), down(d) {}
	~Node() { delete link[1]; delete link[0]; }
	void Display(int, int, int); //����� ���� �� �����
	Node(const Node&) = delete;	//����������� ����������� ��� ����
//	friend class AVL;
};

using MyStack = stack<pair<Node*, int>>;

//�������� ������
//template<class Container = Set>
struct myiter : public std::iterator<std::forward_iterator_tag, int>
{
	Node* Ptr;
	MyStack St;
	myiter(Node* p = nullptr) : Ptr(p) {}
	myiter(Node* p, const MyStack&& St) : Ptr(p), St(move(St)) {}
	bool operator == (const myiter& Other) const { return Ptr == Other.Ptr; }
	bool operator != (const myiter& Other) const { return !(*this == Other); }
	myiter& operator++();
	myiter operator++(int) { myiter temp(*this); ++* this; return temp; }
	pointer operator->() { return &Ptr->key; }
	reference operator*() { return Ptr->key; }
};
//�������� �������
template <typename Container, typename Iter = myiter>
class outiter : public std::iterator<std::output_iterator_tag, typename Container::value_type>
{
protected:
	Container& container;    // ��������� ��� ������� ���������
	Iter iter;	// ������� �������� ���������
public:
	// �����������
	explicit outiter(Container& c, Iter it) : container(c), iter(it) { }

	// ������������ - ������� ����� � ���������
	const outiter<Container>&
		operator = (const typename Container::value_type& value) {
		iter = container.insert(value, iter).first;
		return *this;
	}
	const outiter<Container>& //������������ ����� - ���������
		operator = (const outiter<Container>&) { return *this; }
	// ������������� - ������ ��������
	outiter<Container>& operator* () { return *this; }
	// ��������� - ������ ��������
	outiter<Container>& operator++ () { return *this; }
	outiter<Container>& operator++ (int) { return *this; }
};

// ������� ��� �������� ��������� ������� - �������� ��� ������ ���������
template <typename Container, typename Iter>
inline outiter<Container, Iter> outinserter(Container& c, Iter it)
{
	return outiter<Container, Iter>(c, it);
}
//myiter myiter0(nullptr);

class AVL {	//���-������
	static  size_t tags;

	char tag;
	vector<Node*> mas;
	Node* root;
	size_t h, n; //������ � �������� ������
//	static myiter myiter0;
public: //����������� �������� ����������
	using key_type = int;
	using value_type = int;
	using key_compare = less<int>;
	void swap(AVL& rgt)
	{
		std::swap(tag, rgt.tag);
		std::swap(root, rgt.root);
		std::swap(n, rgt.n);
		std::swap(mas, rgt.mas);
		std::swap(h, rgt.h);
	}
	Node* get_root() { return root; }
	myiter Insert(const int& k,
		myiter where) {
		return insert(k, where).first;
	}
	void Display(int = 1);
	myiter begin()const;
	myiter end()const { return myiter(nullptr); }
	pair<myiter, bool> insert(int, myiter = myiter(nullptr));
	AVL() : tag('A' + tags++), root(nullptr), h(0), n(0) {	}
	AVL(int k, char name) : tag(name), n(k)
	{
		for (int i = 0; i < k; ++i)
			insert(rand() % lim);
	}
	int size() { return n; }
	void make_tree();
	int Power() const;
	void CONCAT(AVL& A, AVL& B);
	void PrepareExcl(const AVL& rgt);
	void PrepareAnd(const AVL& rgt, int k);
	void EXCL(AVL& A, AVL& B);
	void SUBST(AVL& A, AVL& B, int p);
	template<typename MyIt>
	AVL(MyIt, MyIt); //������������ �� �������
	~AVL() { delete root; mas.clear(); }
	myiter find(int)const; //����� �� �����
	AVL(const AVL& rgt) : AVL() {
		for (auto x = rgt.begin(); x != rgt.end(); ++x) insert(*x);
	}
	AVL(AVL&& rgt) : AVL() { swap(rgt); }
	AVL& operator=(const AVL& rgt)
	{
		AVL temp;
		for (auto x : rgt) temp.insert(x);
		swap(temp);
		return *this;
	}
	AVL& operator=(AVL&& rgt)
	{
		swap(rgt); return *this;
	}
	AVL& operator |= (const AVL&);
	AVL operator | (const AVL& rgt) const
	{
		AVL result(*this); return (result |= rgt);
	}
	AVL& operator &= (const AVL&);
	AVL operator & (const AVL& rgt) const
	{
		AVL result(*this); return (result &= rgt);
	}
	AVL& operator -= (const AVL&);
	AVL operator - (const AVL& rgt) const
	{
		AVL result(*this);
		return (result -= rgt);
	}
	AVL& operator ^= (const AVL&);
	AVL operator ^ (const AVL& rgt) const
	{
		AVL result(*this); return (result ^= rgt);
	}
};

