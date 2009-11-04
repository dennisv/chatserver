#include "imexplode.h"

using namespace std;

vector<string> explode(char delimiter, const string original)
{
	vector<string> ret;
	string token;
	istringstream iss(original);
	while(getline(iss, token, delimiter))
	{
		ret.push_back(token);
	}
	return ret;
}

string implode(char glue, const vector<string> pieces)
{
	string ret;
	int size = pieces.size();
	for (int i = 0; i < size; i++)
	{
		ret += pieces[i];
		if (i < (size - 1))
			ret += glue;
	}
	return ret;
}
