#pragma once

//Some utils for working with UE by Kuflex

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>  //ostringsream
#include <iomanip>  //setprecision
using namespace std;

#define KU_PRINT(text) { if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2, FColor::White,string(text).c_str()); } 
#define KU_PRINT_FSTRING(text) { if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2, FColor::White,text); } 
#define KU_LOG(text) UE_LOG(LogTemp, Warning, TEXT(text));


struct ku {
	
	static string toString(FString v) {
		return TCHAR_TO_UTF8(*v);
	}

	template <class T>	//T = AActor, AStaticMeshActor, ...
	static T* find_actor(string name0, UObject * WorldContextObject) {
		UWorld * World = GEngine->GetWorldFromContextObject(WorldContextObject);
		if (World) {
			for (TActorIterator<T> ActorItr(World); ActorItr; ++ActorItr)
			{
				string name = //TCHAR_TO_UTF8(*ActorItr->GetName());
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
	static vector <string> splitString(const string &source, const string &delimiter, bool ignoreEmpty = false) {
		vector<string> result;
		if (delimiter.empty()) {
			result.push_back(source);
			return result;
		}
		string::const_iterator substart = source.begin(), subend;
		while (true) {
			subend = search(substart, source.end(), delimiter.begin(), delimiter.end());
			string sub(substart, subend);
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
	static string toString(const T& value) {
		ostringstream out;
		out << value;
		return out.str();
	} 

	//----------------------------------------
	template <class T>
	static string toString(const T& value, int precision) {
		ostringstream out;
		out << fixed << setprecision(precision) << value;
		return out.str();
	}

	//----------------------------------------
	template <class T>
	static string toString(const T& value, int width, char fill) {
		ostringstream out;
		out << fixed << setfill(fill) << setw(width) << value;
		return out.str();
	}

	//----------------------------------------
	template <class T>
	static string toString(const T& value, int precision, int width, char fill) {
		ostringstream out;
		out << fixed << setfill(fill) << setw(width) << setprecision(precision) << value;
		return out.str();
	}

	//----------------------------------------
	template<class T>
	static string toString(const vector<T>& values) {
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
	static int toInt(const string& intString) {
		int x = 0;
		istringstream cur(intString);
		cur >> x;
		return x;
	}

	//----------------------------------------
	static float toFloat(const string& floatString) {
		float x = 0;
		istringstream cur(floatString);
		cur >> x;
		return x;
	}

	//----------------------------------------
	static double toDouble(const string& doubleString) {
		double x = 0;
		istringstream cur(doubleString);
		cur >> x;
		return x;
	}

	//----------------------------------------
	static char toChar(const string& charString) {
		char x = '\0';
		istringstream cur(charString);
		cur >> x;
		return x;
	}

	//----------------------------------------
	

};
