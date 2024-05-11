#include <iostream>
#include <string>
#include "shell.hpp"
using namespace std;

#define MAX_ARGC 32
#define MAX_ARG_LEN 128
#define LINE_SIZE 512

// #define SHELL_DEBUG

#ifdef SHELL_DEBUG

void carve(const string& file, int width, int height, const string& outfile) {
	cout << "carving " << file << endl;
	cout << "width " << width << endl;
	cout << "height " << height << endl;
	cout << "output " << outfile << endl;
}

#endif

bool parser(string& line, string argv[]) {
	int len = line.size();
	int arg = 0, index = 0;
	for (int i = 0; i < len; ++i) {
		if (line[i] == ' ') {
			arg++;
			if (arg >= MAX_ARGC) {
				cerr << "参数太多" << endl;
				return false;
			}
		}
		else if (line[i] == '\n') {
			return true;
		}
		else {
			argv[arg].push_back(line[i]);
		}
	}
	return true;
}

void shell() {
	string line;
	string argv[MAX_ARGC];
	cout << "输入 \"carve [path]\" 裁剪照片" << endl;
	cout << "在后面跟上 \"-s\" <width> <height> 表示裁剪后的尺寸，缺省值为0，"
		<<"0表示减半，-1表示不变" << endl;
	cout << "在后面跟上 \"-o\" <path> 表示输出文件路径，缺省值为./output.png，"
		<< endl;
	cout << "示例：carve input.png" << endl;
	cout << "carve input.png -s 0 -1 -o ans.png" << endl;
	cout << "输入 \"exit\" 退出" << endl;
	cout << "----------" << endl;	
	while (true) {
		while (cout << ">>> ",getline(cin, line)) {
			for (auto& s : argv) {
				s.clear();
			}
			if (parser(line, argv)==false) {
				continue;
			}
			if (argv[0] == string("exit")) {
				return;
			}
			else if (argv[0] == string("carve")) {
				if (argv[1].empty()) {
					cerr << "Usage: carve [path]" << endl;
					continue;
				}
				string path = argv[1], outfile = "./output.png";
				int width = 0, height = 0;
				for (int i = 2; argv[i].empty() == false;) {
					if (argv[i] == string("-s")) {
						i++;
						try {
							width = stoi(argv[i++]);
							height = stoi(argv[i++]);
						}
						catch (...) {

						}
					}
					else if (argv[i] == string("-o")) {
						i++;
						if (!argv[i].empty()) {
							outfile = argv[i++];
						}
					}
					else ++i;
				}
				seam_carving(path, width, height, outfile);
			}
			else {
				cout << line << endl;
			}
		}
	}
}

#ifdef SHELL_DEBUG

int main() {
	shell();
	return 0;
}

#endif