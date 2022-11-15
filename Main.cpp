#include "PublicLibs/ConsoleIO/BasicIO.h"
#include "PublicLibs/Exceptions/Exception.h"
#include "DigitViewer/DigitViewerUI2.h"

//  Dependencies
#include <algorithm>
#include "PublicLibs/ConsoleIO/BasicIO.h"
#include "PublicLibs/ConsoleIO/Margin.h"
#include "PublicLibs/ConsoleIO/Label.h"
#include "PublicLibs/ConsoleIO/Array.h"
#include "PublicLibs/Exceptions/InvalidParametersException.h"
#include "PublicLibs/BasicLibs/StringTools/ToString.h"
#include "PublicLibs/BasicLibs/Memory/SmartBuffer.h"
#include "PublicLibs/SystemLibs/Concurrency/Parallelizers.h"
#include "PublicLibs/SystemLibs/Environment/Environment.h"
#include "DigitViewer2/Globals.h"
#include "DigitViewer2/PrintHelpers.h"
#include "DigitViewer2/RawToAscii/RawToAscii.h"
#include "DigitViewer2/DigitWriters/BasicDigitWriter.h"
#include "DigitViewer2/DigitWriters/BasicTextWriter.h"
#include "DigitViewer2/DigitWriters/BasicYcdSetWriter.h"
#include "DigitViewer2/DigitViewer/DigitViewerTasks.h"
#include "DigitViewer2/DigitViewer/DigitViewerTasks.h"
#include "DigitViewer2/DigitReaders/BasicDigitReader.h"

#include "PublicLibs/ConsoleIO/BasicIO.h"
#include "PublicLibs/ConsoleIO/Label.h"
#include "PublicLibs/SystemLibs/FileIO/FileException.h"
#include "DigitViewer2/Globals.h"
#include "DigitViewer2/DigitReaders/BasicDigitReader.h"
#include "DigitViewer2/DigitReaders/BasicTextReader.h"
#include "DigitViewer2/DigitReaders/BasicYcdSetReader.h"

#ifdef YMP_STANDALONE
#include "PrivateLibs/SystemLibs/ParallelFrameworks/ParallelFrameworks.h"
#endif

#include <iostream>
//#include "DigitViewer2/BigInt.hpp"
#include <string.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <filesystem>
#include <map>
#include <random>
#include <cstdio>
#include <thread>

using namespace std;

#define N_BASE_FOR_SEARCH 23
#define DELETE_FILE_AFTER_RUN true

void loadInterval(DigitViewer2::BasicYcdSetReader &reader, unsigned long long int  start, unsigned long long int  digits, string& str) {
	using DigitViewer2::BUFFER_ALIGNMENT;
	DigitViewer2::upL_t bytes = reader.recommend_buffer_size(digits, -1);
	DigitViewer2::SmartBuffer<> buffer(bytes, BUFFER_ALIGNMENT);

	//  Read digits
	str.resize(digits, '-');
	reader.load_digits(
		&str[0], nullptr,
		start, digits,
		DigitViewer2::AlignedBufferC<BUFFER_ALIGNMENT>(buffer, bytes),
		DigitViewer2::parallelizer_none, 1
	);

	DigitViewer2::RawToAscii::raw_to_dec(&str[0], &str[0], digits);
}

bool inline checkPalindrome(string& pi, int cL, int cR) {
	while (cL < cR) {
		if (pi[cL] != pi[cR]) {
			return false;
		}
		cL++; cR--;
	}
	return true;
}
#define MIN(a,b) ((a) < (b) ? (a) : (b))

int main() {
	const unsigned long long int totalFileSize = 100000000000;
	const unsigned long long int debugChunks = totalFileSize / 10;
	std::ofstream* filestream_all;

	filestream_all = new ofstream("output_palindrom_all_run.csv", std::ios_base::app);
	//*filestream_all << "position,palindrome,length, file\n";
	//while (true) {
		// Wait for files
		cout << "waiting for files" << endl;
		for (const auto& entry : std::filesystem::directory_iterator("pi")) {
			string inputToLoad = entry.path().string();
			string number = inputToLoad.substr(inputToLoad.size() - 7, 3);

			string extension = inputToLoad.substr(inputToLoad.size() - 4, 4);
			if (extension != ".ycd") {
				continue;
			}
			std::cout << entry.path() << std::endl;

			unsigned long long int loadedDigits = 0;


			std::ofstream* filestream;
			filestream = new ofstream("output\\output_palindrom_" + number + ".csv", ios::out);
			*filestream << "position,palindrome,length\n";

			int fileIndex = atoi(number.c_str());
			unsigned long long int start = totalFileSize * fileIndex;
			const unsigned long long int littleCry = 150;
			const unsigned long long int digits = 100000000;

			DigitViewer2::BasicYcdSetReader* reader = new DigitViewer2::BasicYcdSetReader(inputToLoad);

			std::cout << "Will load Text " << entry.path() << endl;
			while (loadedDigits < totalFileSize) {
				string pi;
				unsigned long long int bufferToLoad = MIN(digits + littleCry, (totalFileSize - loadedDigits));
				loadInterval(*reader, start, bufferToLoad, pi);
				loadedDigits += bufferToLoad;
				const unsigned long long int size = pi.size();

				//std::cout << "loaded Text " << entry.path() << " lines " << size << endl;
				fflush(stdout);

#pragma omp parallel
#pragma omp for
				for (int i = 0; i < size - N_BASE_FOR_SEARCH; i++) {
					unsigned int j = i + N_BASE_FOR_SEARCH - 1;
					for (int odd = 0; odd < 2; odd++) {
						j = j + odd;

						if (pi[i] != pi[j]) {
							j = j - odd;
							continue;
						}

						if (checkPalindrome(pi, i, j)) {
							fflush(stdout);

							//----- Expand <-->?
							unsigned long long int local_i, local_j;
							local_i = i;
							local_j = j;
							while (pi[local_i - 1] == pi[local_j + 1]) {
								local_i--;
								local_j++;
							}

							const string n = pi.substr(local_i, (local_j - local_i) + 1);
							const unsigned long long position = (start + i) - (i - local_i) - 2;
							cout << "Pos " << position << endl;
							const int length = (local_j - local_i) + 1;
							cout << "Len " << length << endl;
							cout << "Pal " << n << endl;
							cout << endl;

							/// gambs
							const string aux(to_string(position));
							stringstream stream;
							stream << aux;

							// Save to CSV
							*filestream << stream.str() + "," + n + "," + to_string(length) + "\n";
							filestream->flush();

							*filestream_all << stream.str() + "," + n + "," + to_string(length) + "," + inputToLoad + "\n";
							filestream_all->flush();
						}

						// Important
						j = j - odd;
					}
				}
				start += digits;
			}

			if (filestream) {
				std::cout << "closing..." << endl;
				filestream->close();
			}

			delete reader;
			if (DELETE_FILE_AFTER_RUN) {
				std::filesystem::remove(inputToLoad);
			}
		}

	//	std::this_thread::sleep_for(std::chrono::seconds(60));
	//}

	if (filestream_all) {
		std::cout << "closing all..." << endl;
		filestream_all->close();
	}
}





