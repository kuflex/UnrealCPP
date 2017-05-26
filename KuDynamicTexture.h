// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"


//-----------------------------------------------------------------------
//Based on https://wiki.unrealengine.com/Procedural_Materials
//makuto/DynamicTexture.cpp https://gist.github.com/makuto/bf87e5ccd0b15b0859608c9b745ac5f1

//-----------------------------------------------------------------------
struct DynamicTexturePixel
{
	// Do not rearrage or add members to this struct
	uint8 b;
	uint8 g;
	uint8 r;
	uint8 a;
	static DynamicTexturePixel make(uint8 r0, uint8 g0, uint8 b0, uint8 a0) {
		DynamicTexturePixel p;
		p.r = r0;
		p.g = g0;
		p.b = b0;
		p.a = a0;
		return p;
	}
};


struct KuDynamicTexture
{

public:
	KuDynamicTexture();
	~KuDynamicTexture();

	void setup(UStaticMeshComponent* staticMesh, int32 materialIndex, 
		string dynamicMaterialName,
		int32 width, int32 height);
	void setImageGrayscale(unsigned char *pixels, int w, int h);	//resizes image to texture's dimensions

	void setColor(DynamicTexturePixel color);
	void setTestPattern(int frame);
	void setTestMaskHuman(int frame);

private:
	TArray<class UMaterialInstanceDynamic*> DynamicMaterials;
	UTexture2D* Texture2D;
	FUpdateTextureRegion2D* UpdateTextureRegion;

	bool ready_;
	int32 n_;
	int32 w_;
	int32 h_;
	DynamicTexturePixel* pixels_;

	void updateTexture();	//call this after changing pixels to have effect

};


