#include "CONT.h"
#include <iostream>
#include <iomanip>
#include <stack>
#include<ctime>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <locale>
using std::cout;

myiter AVL::begin()const {
	MyStack St;
	Node* p(root);
	if (p) {
		while (p->link[0]) {
			St.push(make_pair(p, 0));
			p = p->link[0];
		}
	}
	return myiter(p, move(St));
}
myiter& myiter::operator++()
{
	if (!Ptr) { //Первое обращение?
		return *this; //Не работает без предварительной установки на дерево
	}
	if (Ptr->link[1]) { //Шаг вправо
		St.push(make_pair(Ptr, 1));
		Ptr = Ptr->link[1];
		while (Ptr->link[0]) { //... и искать крайний левый
			St.push(make_pair(Ptr, 0));
			Ptr = Ptr->link[0];
		}
	}
	else {
		pair<Node*, int> pp(Ptr, 1);
		while (!St.empty() && pp.second) { pp = St.top(); St.pop(); }
		if (pp.second) { //Шаг вправо - тупик, завершить!
			Ptr = nullptr;
		}
		else Ptr = pp.first; //Шаг вправо и продолжать
	}
	return (*this);
}

const int FIRSTROW = 0, FIRSTCOL = 60,	//Параметры вывода на экран
MAXCOL = 120, OFFSET[] = { 60, 23, 12, 6, 3, 2, 1 },
MAXROW = FIRSTROW + 7, //18
MAXOUT = FIRSTROW + 6, SHIFT = 2;
string sb{ "-o+" };
string SCREEN[MAXROW]; //(MAXROW * MAXCOL);
int row = 0, col = 0;
void gotoxy(int c, int r) { row = r, col = c; }

void clrscr(int f = 1)
{
	for (auto i = 0; i < MAXROW; ++i) {
		SCREEN[i].clear();
		SCREEN[i].resize(MAXCOL + 20, '.');
	}
	if (f) system("cls");
}

void showscr()
{
	for (auto i = 0; i < MAXROW; ++i) {
		SCREEN[i].resize(MAXCOL);
		cout << SCREEN[i] << endl;
	}
}

int setval(string& s, int pos, int val) { //Аккуратная вставка числа в строку
	string t(to_string(val));
	for (auto p : t) s[pos++] = p;
	return t.size();
}

void AVL::Display(int first)
{
	clrscr(first);  // Вызов для главного дерева -> очистить экран
	SCREEN[0].replace(20, 11, "AVL-Tree(h=");
	setval(SCREEN[0], 31, h);
	SCREEN[0].replace(34, 3, " n=");
	setval(SCREEN[0], 37, n);
	SCREEN[0].replace(40, 10, ") --------->");
	if (root) root->Display(0, FIRSTCOL, 1);
	else SCREEN[0].replace(77, 9, "<Empty!>");
	showscr();
	cout << "<";
	for (auto x : mas) cout << x->key << " "; cout << ">\n";
}

void Node::Display(int row, int col, int depth) //Вывод узла в точку (row,col)
{
	if ((row > MAXROW) || (col < 0) || (col > MAXCOL)) return;
	gotoxy(col, row);
	if (row > MAXOUT) {
		SCREEN[row].replace(col, col + 3, "+++");
		return;
	}
	//	SCREEN[row].replace(col, col+5, to_string(key));
	try {
		setval(SCREEN[row], col, key);
		SCREEN[row + 1][col] = sb[b + 1];
	}
	catch (exception& e) {
		cout << e.what() << key << ' ' << row << ' ' << col << endl;
		cin.get();
	}
	catch (...) { cout << "Unknown error\n"; cin.get(); }
	if (link[0]) link[0]->Display(row + 1, col - OFFSET[depth], depth + 1);
	if (link[1]) link[1]->Display(row + 1, col + OFFSET[depth], depth + 1);
}
myiter AVL::find(int k)const
{
	Node* p(root);
	while (p && p->key != k) p = p->link[p->key > k];
	return myiter(p);
}

pair<myiter, bool> AVL::insert(int k, myiter where)//Вставка нового ключа в дерево
{
	int a(0), B[]{ -1, +1 }; int cont = 1;
	MyStack St(move(where.St));
	//===== Инициализация =====
	if (!where.Ptr) { //Вставка в пустое дерево или свободная
		if (!root) { // Дерево пусто
			root = new Node(k); //Создать корень
			n = h = 1; //...мощность и высота = 1
			mas.push_back(root);
			return make_pair(myiter(root, move(St)), true);
			//...выход: "вставлено"
		}
	}
	Node* p(where.Ptr ? where.Ptr : root),
		* q(nullptr);
	//===== Поиск места вставки =====
	while (k != p->key && cont)
	{
		a = k > p->key ? 1 : 0;
		St.push(make_pair(p, a));
		q = p->link[a];
		if (q) { //Есть сын
			p = q; //Шаг вниз
		}
		else {
			cont = 0;
			p->link[a] = q = new Node(k); //Вставка
			++n;
			break;
		}
	}
	if (!cont)
		mas.push_back(q);
	else
	{
		mas.push_back(p);
		return make_pair(myiter(q), false); //Элемент уже имеется
	}
	//Вставлен новый узел в точке q. Корректировка балансов на пути вверх до первого ненулевого
	while (!St.empty())
	{
		auto pa = St.top(); St.pop();
		p = pa.first; a = pa.second;
		if (!p->b) {
			p->b = B[a]; //Замена 0 на +-1
			if (p == root) //Дерево подросло
			{
				++h; break;
			}
			else q = p, p = St.top().first; //Шаг вверх, цикл продолжается
		}
		else if (p->b == -B[a]) { //Сбалансировалось
			p->b = 0; //Замена +-1 на 0
			break;
		}
		//===== Перебалансировка =====
		else if (p->b == q->b) { //Случай I: Однократный поворот
			p->link[a] = q->link[1 - a];
			q->link[1 - a] = p;
			p->b = q->b = 0;
			if (p == root)p = root = q;
			else St.top().first->link[St.top().second] = p = q;
			break;
		}
		else { //Случай II: Двукратный поворот
			Node* r(q->link[1 - a]);
			p->link[a] = r->link[1 - a];
			q->link[1 - a] = r->link[a];
			r->link[1 - a] = p;
			r->link[a] = q;
			if (r->b == B[a]) { p->b = -B[a]; q->b = 0; }
			else if (r->b == -B[a]) { p->b = 0; q->b = B[a]; }
			else { p->b = q->b = 0; }
			r->b = 0;
			if (p == root) p = root = r;
			else St.top().first->link[St.top().second] = p = r;
			break;
		}
	}
	return make_pair(myiter(p, move(St)), true);
}


void AVL::make_tree()
{
	myiter it; bool f;
	pair<myiter, bool> a;
	a = make_pair(it, f);
	int k = 1 + rand() % 25;
	for (int i = 0; i < k; ++i) insert(rand() % 25 + 1);
}

int AVL::Power() const { return mas.size(); };
void AVL::PrepareExcl(const AVL& rgt)
{
	int a = rand() % rgt.Power();
	int b = rand() % rgt.Power();
	if (b < a)
		std::swap(a, b);
	for (int x = a; x <= b; ++x)
	{
		int y = rgt.mas[x]->key;
		insert(y);
	}
}
void AVL::PrepareAnd(const AVL& rgt, int k) {
	for (int i = 0; i < k; ++i)
	{ //Подготовка пересечения:
		  //добавление общих эл-тов
		int y = rgt.mas[i]->key;
		insert(y);
	}
}

void AVL::CONCAT(AVL& A, AVL& B)
{
	AVL temp;
	vector<Node*>::iterator iter;
	for (iter = A.mas.begin(); iter != A.mas.end(); ++iter)
		temp.insert((*iter)->key);
	for (iter = B.mas.begin(); iter != B.mas.end(); ++iter)
		temp.insert((*iter)->key);
	swap(temp);
}


void AVL::EXCL(AVL& A, AVL& B)
{
	int i = 0; auto k = B.mas.begin();
	vector<Node*>::iterator iter;
	for (auto iterator = A.mas.begin(); iterator != A.mas.end() && i < B.mas.size(); ++iterator)
	{
		if ((*iterator)->key == (*k)->key)
		{
			if (!i)
				iter = iterator;
			++i;
			++k;
		}
		else
		{
			k = B.mas.begin();
			i = 0;
		}
	}
	AVL temp;
	if (i)
	{
		for (auto iterator = A.mas.begin(); iterator != iter; ++iterator)
			temp.insert((*iterator)->key);
		for (auto iterator = iter + B.mas.size(); iterator != A.mas.end(); ++iterator)
			temp.insert((*iterator)->key);
	}
	swap(temp);
}
void AVL::SUBST(AVL& A, AVL& B, int p)
{
	vector<Node*>::iterator iter; int i = 0;
	AVL temp;
	if (p > A.Power())
		(*this).CONCAT(A, B);
	else
	{
		for (iter = A.mas.begin(); iter != A.mas.end() && i < p; ++i, ++iter)
			temp.insert((*iter)->key);
		for (iter = B.mas.begin(); iter != B.mas.end(); ++iter)
			temp.insert((*iter)->key);
		for (iter = A.mas.begin() + p; iter != A.mas.end(); ++iter)
			temp.insert((*iter)->key);
	}
	swap(temp);
}

AVL& AVL::operator |= (const AVL& rgt) {
	AVL temp;
	set_union(begin(), end(), rgt.begin(), rgt.end(), outinserter(temp, myiter(nullptr)));
	swap(temp);
	return *this;
}

AVL& AVL::operator &= (const AVL& rgt) {
	AVL temp;
	set_intersection(begin(), end(), rgt.begin(), rgt.end(), outinserter(temp, myiter(nullptr)));
	swap(temp);
	return *this;
}

AVL& AVL::operator -= (const AVL& rgt) {
	AVL temp;

	set_difference(begin(), end(), rgt.begin(), rgt.end(), outinserter(temp, myiter(nullptr)));
	swap(temp);
	return *this;
}
AVL& AVL::operator ^= (const AVL& rgt) {
	AVL temp;
	set_symmetric_difference(begin(), end(), rgt.begin(), rgt.end(), outinserter(temp, myiter(nullptr)));
	swap(temp);
	return *this;
}
size_t AVL::tags = 0;


int main()
{
	using std::cout;
	using namespace std::chrono;
	setlocale(LC_ALL, "Russian");
	//srand((unsigned int)7); //Пока здесь константа, данные повторяются
	srand((unsigned int)time(nullptr)); //Разблокировать для случайных данных
	bool debug = false; //false, чтобы запретить отладочный вывод
	auto MaxMul = 5;
	int middle_power = 0, set_count = 0;
	auto Used = [&](AVL& t){ middle_power += t.Power();
	++set_count; };
	auto DebOut = [debug](AVL& t) { if (debug) {
		t.Display(0);
		system("pause");
	}};
	auto rand = [](int d) { return std::rand() % d; }; //Лямбда-функция!
	ofstream fout("in.txt"); //Открытие файла для результатов
	//int p = rand(20) + 1;
	for (int p = 6; p <= 402; p = p + 2) //Текущая мощность (место для цикла по p)
	{
		//=== Данные ===
		AVL A(p, 'A');
		AVL B(p, 'B');
		AVL C(p, 'C');
		AVL D(p, 'D');
		AVL E(p, 'E');
		AVL RES1, RES2, RES3, RES4, CON, EX, RESEX, SUB;
		try
		{
			auto t1 = std::chrono::high_resolution_clock::now();
			if (debug) cout << "\nOperations with set:\n";
			int q_sub(rand(MaxMul) + 1); 
			C.PrepareAnd(B, q_sub); middle_power += q_sub;
			D.PrepareAnd(B & C, q_sub); middle_power += q_sub;
			if (debug) cout << "\nB:\n"; DebOut(B); Used(B);
			if (debug) cout << "\nC:\n"; DebOut(C); Used(C);
			RES1 = B & C; if (debug) cout << "\nB&C:\n"; DebOut(RES1); Used(RES1);
			if (debug) cout << "\nD\n:"; DebOut(D), Used(D);
			RES2 = RES1 & D; if (debug) cout << "\nB&C&D:\n"; DebOut(RES2); Used(RES2);
			if (debug) cout << "\nE\n:"; DebOut(E); Used(E);
			RES3 = RES2 ^ E;  if (debug) cout << "\nB&C&D^E:\n"; DebOut(RES3); Used(RES3);
			if (debug) cout << "\nA\n:"; DebOut(A); Used(A);
			RES4 = A - RES3; if (debug) cout << "\nA-(B&C&D)^E:\n"; DebOut(RES4); Used(RES4);
			if (debug)
				cout << "\nOperations with sequences:\n";
			if (debug) cout << "\nB:\n"; DebOut(B); 
			if (debug) cout << "\nC:\n"; DebOut(C); 
			CON.CONCAT(B, C); if (debug) cout << "\nCONCAT(B,C):\n"; DebOut(CON); Used(CON);
			EX.PrepareExcl(E); if (debug) cout << "\EX:\n"; DebOut(EX); Used(EX);
			if (debug) cout << "\nE\n:"; DebOut(E);
			RESEX.EXCL(E, EX); if (debug) cout << "\EXCL(E,EX):\n"; DebOut(RESEX); Used(RESEX);
			if (debug) cout << "\nA\n:"; DebOut(A); 
			if (debug) cout << "\nB:\n"; DebOut(B); 
			int r = rand(A.Power());
			if (debug)
				cout << "\nNumber of position is " << r << endl;
			SUB.SUBST(A, B, r); if (debug) cout << "\SUBST(A,B):\n"; DebOut(SUB); Used(SUB);
			auto t2 = std::chrono::high_resolution_clock::now();
			auto dt = duration_cast <duration<double>>(t2 - t1);
			int mp = middle_power / set_count;
			fout << mp << ' ' << dt.count() << endl;
		}
		catch(...) { }
		
	}
	cout << "\n===КОНЕЦ===";
	cin.get();
	return 0;
}
