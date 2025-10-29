// cl.exe reference.cpp

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main()
{
	vector<string> array;

	array.push_back("aaaaaaa");
	array.push_back("bbbbbbb");
	array.push_back("ccccccc");

	string& ref = array[0];

	cout << ref.length() << ", " << ref << endl;

	array.push_back("ddddddd");

	cout << ref.length() << ", " << ref << endl;
}
