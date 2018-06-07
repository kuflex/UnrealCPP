#pragma once

//Some utils for working with UE by Kuflex
#include "CoreMinimal.h"
#include "FileHelper.h"
#include "FileManagerGeneric.h"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>  //ostringsream
#include <iomanip>  //setprecision
#include <fstream>



//std::std::string  namespace std;

extern bool KU_PRINT_ENABLED;	//should be declared somethere for controlling printing to screen

#define KU_PRINT(text) { if (KU_PRINT_ENABLED && GEngine) GEngine->AddOnScreenDebugMessage(-1, 2, FColor::White,std::string(text).c_str()); } 
#define KU_PRINT_FSTRING(text) { if (KU_PRINT_ENABLED && GEngine) GEngine->AddOnScreenDebugMessage(-1, 2, FColor::White,text); } 
#define KU_LOG(text) UE_LOG(LogTemp, Warning, TEXT(text));

#define KU_DEG_TO_RAD (3.1415926535 / 180.0)

struct ku {
	
	static std::string toString(FString v) {
		return TCHAR_TO_UTF8(*v);
	}

	template <class T>	//T = AActor, AStaticMeshActor, ...
	static T* find_actor(std::string name0, UObject * WorldContextObject) {
		UWorld * World = GEngine->GetWorldFromContextObject(WorldContextObject);
		if (World) {
			for (TActorIterator<T> ActorItr(World); ActorItr; ++ActorItr)
			{
				std::string name = //TCHAR_TO_UTF8(*ActorItr->GetName());
					TCHAR_TO_UTF8(*UKismetSystemLibrary::GetDisplayName(*ActorItr));
				if (name == name0) {
					return *ActorItr;
				}
			}
		}
		return 0;
	}
	//----------------------------------------
	static float map(float t, float a, float b, float A, float B, bool clamp0 = false) {
		if (a == b) return (A + B) / 2;
		t = A + (B - A)*(t - a) / (b - a);
		if (clamp0) t = clamp(t, A, B);
		return t;
	}

	//----------------------------------------
	static float clamp(float t, float a, float b) {
		if (t < a) t = a;
		if (t > b) t = b;
		return t;
	}

	

	//----------------------------------------
	//These functions are copied from openFrameworks source code http://openframeworks.cc
	static std::vector <std::string> splitString(const std::string &source, const std::string &delimiter, bool ignoreEmpty = false) {
		std::vector<std::string> result;
		if (delimiter.empty()) {
			result.push_back(source);
			return result;
		}
		std::string::const_iterator substart = source.begin(), subend;
		while (true) {
			subend = search(substart, source.end(), delimiter.begin(), delimiter.end());
			std::string sub(substart, subend);
			if (!ignoreEmpty || !sub.empty()) {
				result.push_back(sub);
			}
			if (subend == source.end()) {
				break;
			}
			substart = subend + delimiter.size();
		}
		return result;
	}

	//----------------------------------------
	template <class T>
	static std::string toString(const T& value) {
		std::ostringstream out;
		out << value;
		return out.str();
	} 

	//----------------------------------------
	template <class T>
	static std::string toString(const T& value, int precision) {
		std::ostringstream out;
		out << fixed << setprecision(precision) << value;
		return out.str();
	}

	//----------------------------------------
	template <class T>
	static std::string toString(const T& value, int width, char fill) {
		std::ostringstream out;
		out << fixed << setfill(fill) << setw(width) << value;
		return out.str();
	}

	//----------------------------------------
	template <class T>
	static std::string toString(const T& value, int precision, int width, char fill) {
		std::ostringstream out;
		out << fixed << setfill(fill) << setw(width) << setprecision(precision) << value;
		return out.str();
	}

	//----------------------------------------
	template<class T>
	static std::string toString(const std::vector<T>& values) {
		stringstream out;
		int n = values.size();
		out << "{";
		if (n > 0) {
			for (int i = 0; i < n - 1; i++) {
				out << values[i] << ", ";
			}
			out << values[n - 1];
		}
		out << "}";
		return out.str();
	}

	//----------------------------------------
	static int toInt(const std::string& intString) {
		int x = 0;
		std::istringstream cur(intString);
		cur >> x;
		return x;
	}

	//----------------------------------------
	static float toFloat(const std::string& floatString) {
		float x = 0;
		std::istringstream cur(floatString);
		cur >> x;
		return x;
	}

	//----------------------------------------
	static double toDouble(const std::string& doubleString) {
		double x = 0;
		std::istringstream cur(doubleString);
		cur >> x;
		return x;
	}

	//----------------------------------------
	static char toChar(const std::string& charString) {
		char x = '\0';
		std::istringstream cur(charString);
		cur >> x;
		return x;
	}

	//----------------------------------------
	static bool fileExists(std::string fileName) {
		std::ifstream inp;
		inp.open(fileName.c_str(), std::ifstream::in);
		inp.close();
		return !inp.fail();
	}

	//----------------------------------------
	static std::vector<std::string> scan_folder(std::string folder, std::string mask = "*.*", bool search_folders = false) {
		TArray<FString> FileNames;
		FFileManagerGeneric FileMgr;
		FileMgr.SetSandboxEnabled(true);// don't ask why, I don't know :P
		FString wildcard(ANSI_TO_TCHAR(mask.c_str())); // May be "" (empty std::string) to search all files
								   //FString search_path(FPaths::Combine(*FPaths::GameDir(), TEXT("Data"), *wildcard));

		FString search_path(FPaths::Combine(ANSI_TO_TCHAR(folder.c_str()), *wildcard));
		//KU_PRINT(TCHAR_TO_ANSI(*search_path));

		FileMgr.FindFiles(FileNames, *search_path,
			!search_folders,  // to list files
			search_folders); // to skip directories

		std::vector<std::string> files;
		for (auto f : FileNames)
		{
			FString filename(f);
			//f.RemoveFromEnd(ANSI_TO_TCHAR(remove_extension));   //".xml");
			//KU_PRINT(TCHAR_TO_ANSI(*f));
			files.push_back(TCHAR_TO_ANSI(*f));
			//OutputDebugStringA(TCHAR_TO_ANSI(*(f + "\n")));
		}

		FileNames.Empty();// Clear array

		return files;
	}

	//----------------------------------------
	static std::vector<std::string> read_strings(std::string fileName) {
		//kuAssert(kuFileExists(fileName), "read_strings no file " + fileName);
		std::vector<std::string> lines;
		std::ifstream f(fileName.c_str(), std::ios::in | std::ios::binary);
		std::string line;
		while (getline(f, line)) {
			if (line == "") continue;
			else {
				//убираем в конце '\r' для правильного считывания windows-файлов в linux
				//и в windows также сейчас такие есть
				while (!line.empty() && line[line.length()-1] == '\r') {
					line = line.substr(0, line.length() - 1);
				}
				lines.push_back(line);
			}
		}
		return lines;
	}

	//----------------------------------------
	static void write_strings(const std::vector<std::string> &list, std::string fileName) {
		std::ofstream f(fileName.c_str(), std::ios::out);
		//kuAssert(!f.fail(), "write_strings - error creating file " + fileName);
		for (size_t i = 0; i<list.size(); i++) {
			f << list[i] << std::endl;
		}
		//kuAssert(!f.fail(), "write_strings - error writing file " + fileName);
		f.close();
	}

};
