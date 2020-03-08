const char* license = "(In short, Diff Template Library is distributed under so called \"BSD license\",\
\
Copyright(c) 2015 Tatsuhiko Kubo <cubicdaiya@gmail.com>\
All rights reserved.\
\
Redistribution and use in source and binary forms, with or without modification,\
are permitted provided that the following conditions are met :\
\
*Redistributions of source code must retain the above copyright notice,\
this list of conditions and the following disclaimer.\
\
* Redistributions in binary form must reproduce the above copyright notice,\
this list of conditions and the following disclaimer in the documentation\
and/or other materials provided with the distribution.\
\
* Neither the name of the authors nor the names of its contributors\
may be used to endorse or promote products derived from this software\
without specific prior written permission.\
\
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\
\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\
A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT\
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED\
	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING\
		NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.)";

#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>
#include "../dtl-master/dtl/dtl.hpp"

std::vector<unsigned char> cache;

#	define PRINT(color,format,...) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);printf(format,##__VA_ARGS__); }

#define WHITE 15
#define RED 12
#define GREEN 2

template <typename sesElem, typename stream>
class InlinePrinter : public dtl::ChangePrinter< sesElem, stream >
{
	bool print_delete = true;

public:
	InlinePrinter() : dtl::ChangePrinter<sesElem, stream>() {}
	InlinePrinter(stream& out, bool print_delete = true) : dtl::ChangePrinter<sesElem, stream>(out), print_delete(print_delete) {}
	~InlinePrinter() {}

	void operator() (const sesElem& se) const override
	{
		using namespace dtl;
		switch (se.second.type) {
		case SES_ADD:
			PRINT(RED, "%02x ", se.first & 0xff);
			break;
		case SES_DELETE:
			if (!print_delete)
				break;
			PRINT(GREEN, "%02x ", se.first & 0xff);
			break;
		case SES_COMMON:
			PRINT(WHITE, "%02x ", se.first & 0xff);
			break;
		default:
			break;
		}
	}
};

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("RealTimeFileDiff - enter file you wish to track");
		return 0;
	}

	while (1) {
		Sleep(1000);

		std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<unsigned char> buffer(size);
		if (!file.read((char*)buffer.data(), size))
		{
			return -1;
		}

		if (cache.empty())
		{
			for (auto c : buffer) {
				PRINT(WHITE, "%02x ", c & 0xff);
			}
			cache = buffer;
		}
		else {
			auto different = false;
			if (buffer.size() != cache.size())
				different = true;
			else {
				for (unsigned i = 0; i < buffer.size(); i++) {
					if (buffer[i] != cache[i]) {
						different = true;
						break;
					}
				}
			}

			if (different) {
				system("cls");

				dtl::Diff< unsigned char, std::vector<unsigned char> > d(buffer, cache);
				d.compose();
				auto ses = d.getSes();
				auto ses_v = ses.getSequence();
				std::for_each(ses_v.begin(), ses_v.end(), InlinePrinter< std::pair<unsigned char, dtl::elemInfo>, decltype(std::cout)>(std::cout));

				cache = buffer; 
			}
		}
	}
	return 1;
}