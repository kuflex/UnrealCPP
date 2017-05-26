// Fill out your copyright notice in the Description page of Project Settings.

#include "KuNetTexture.h"
#include "KuDynamicTexture.h"
#include "KuUtils.h"


//----------------------------------------------------------------------------------------------------------------------
struct UpdateTextureRegionsParams
{
	UTexture2D* Texture;
	int32 MipIndex;
	uint32 NumRegions;
	FUpdateTextureRegion2D* Regions;
	uint32 SrcPitch;
	uint32 SrcBpp;
	uint8* SrcData;
	bool FreeData;
};

// Send the render command to update the texture
void UpdateTextureRegions(UpdateTextureRegionsParams params)
{
	if (params.Texture && params.Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)params.Texture->Resource;
		RegionData->MipIndex = params.MipIndex;
		RegionData->NumRegions = params.NumRegions;
		RegionData->Regions = params.Regions;
		RegionData->SrcPitch = params.SrcPitch;
		RegionData->SrcBpp = params.SrcBpp;
		RegionData->SrcData = params.SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData, FUpdateTextureRegionsData*, RegionData, RegionData, bool,
			FreeData, params.FreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex], RegionData->SrcPitch,
							RegionData->SrcData +
							RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch +
							RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp);
					}
				}
		if (FreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});
	}
}

//----------------------------------------------------------------------------------------------------------------------
KuDynamicTexture::KuDynamicTexture() {
	ready_ = false;
}

//----------------------------------------------------------------------------------------------------------------------
KuDynamicTexture::~KuDynamicTexture() {
	if (pixels_) delete[] pixels_;
	if (UpdateTextureRegion) delete UpdateTextureRegion;
}

//----------------------------------------------------------------------------------------------------------------------
void KuDynamicTexture::setup(UStaticMeshComponent* staticMesh, int32 materialIndex, 
	string dynamicMaterialName,
	int32 width, int32 height) {
	if (ready_) {
		KU_PRINT("ERROR KuDynamicTexture::setup: already inited, but called second time!");
		return;
	}
	if (!staticMesh) {
		KU_PRINT("ERROR KuDynamicTexture::setup: NULL staticMesh");
		return;
	}

	w_ = width;
	h_ = height;


	if (w_ <= 0 || h_ <= 0)
	{
		KU_PRINT("ERROR: KuDynamicTexture::setup - bad width, height");
		return;
	}

	// Texture2D setup
	{
		Texture2D = UTexture2D::CreateTransient(w_, h_);
		// Ensure there's no compression (we're editing pixel-by-pixel)
		Texture2D->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		// Turn off Gamma correction
		Texture2D->SRGB = 0;
		// Make sure it never gets garbage collected
		Texture2D->AddToRoot();
		// Update the texture with these new settings
		Texture2D->UpdateResource();
	}

	// TextureRegion setup
	UpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, w_, h_);

	// Pixels setup
	n_ = w_ * h_;
	pixels_ = new DynamicTexturePixel[n_];
	
	// Material setup
	UMaterialInstanceDynamic* material =
		staticMesh->CreateAndSetMaterialInstanceDynamic(materialIndex);

	if (!material) {
		KU_PRINT("ERROR KuDynamicTexture::setup : NULL material");
		return;
	}

	DynamicMaterials.Empty();
	DynamicMaterials.Add(material);			//Trick to avoid Garbage Collector

	for (UMaterialInstanceDynamic* dynamicMaterial : DynamicMaterials)
	{
		if (dynamicMaterial)
			dynamicMaterial->SetTextureParameterValue(dynamicMaterialName.c_str(), Texture2D);
	}


	ready_ = true;
	
	//setTestPattern(0);
	setColor({ 0, 0, 0, 255 });
}

//----------------------------------------------------------------------------------------------------------------------
void KuDynamicTexture::updateTexture() {
	if (ready_) {
		UpdateTextureRegionsParams params = {
			/*Texture = */ Texture2D,
			/*MipIndex = */ 0,
			/*NumRegions = */ 1,
			/*Regions = */ UpdateTextureRegion,
			/*SrcPitch = */ static_cast<uint32>(w_ * sizeof(DynamicTexturePixel)),
			/*SrcBpp = */ sizeof(DynamicTexturePixel),
			/*SrcData = */ reinterpret_cast<uint8*>(pixels_),
			/*FreeData = */ false,
		};
		UpdateTextureRegions(params);
	}
}


//----------------------------------------------------------------------------------------------------------------------
void KuDynamicTexture::setColor(DynamicTexturePixel color) {
	if (ready_) {
		for (int i = 0; i < n_; i++) {
			pixels_[i] = color;
		}
		updateTexture();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void KuDynamicTexture::setImageGrayscale(unsigned char *pixels, int w, int h) {	//resizes image to texture's dimensions
	if (ready_) {
		for (int y = 0; y < h_; y++) {
			for (int x = 0; x < w_; x++) {
				int x1 = x * w / w_;
				int y1 = y * h / h_;
				uint8 c = pixels[x1 + w*y1];
				pixels_[x + w_ * y] = DynamicTexturePixel::make(c, c, c, 255);
			}
		}
		updateTexture();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void KuDynamicTexture::setTestPattern(int frame) {
	if (ready_) {
		for (int y = 0; y < h_; y++) {
			for (int x = 0; x < w_; x++) {
				pixels_[x + w_ * y] = DynamicTexturePixel::make(((x/10)%2) * 255, ((y/10)%2)*255, (x+y+frame)%256, 255);
			}
		}
		updateTexture();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void KuDynamicTexture::setTestMaskHuman(int frame) {
	if (ready_) {
		for (int y = 0; y < h_; y++) {
			for (int x = 0; x < w_; x++) {
				pixels_[x + w_ * y] = DynamicTexturePixel::make(0, 0, 0, 255);
			}
		}
		int x0 = ku::map(sin(frame*0.01), -1, 1, w_*0.25, w_*0.75);
		int rx = w_ * 0.015;
		int y0 = h_ * 0.3;
		int y1 = h_;

		int z = ku::map(sin(frame*0.005), -1, 1, 64, 255);

		for (int y = y0; y < y1; y++) {
			for (int x = x0 - rx; x < x0 + rx; x++) {
				pixels_[x + w_ * y] = DynamicTexturePixel::make(z, z, z, 255);
			}
		}
		updateTexture();
	}
}

//----------------------------------------------------------------------------------------------------------------------
/*DynamicTexturePixel* DynamicTexture::GetPixel(int32 row, int32 column)
{
	if (!Pixels || row >= Height || column >= Width)
		return nullptr;

	return &Pixels[(row * Width) + column];
}*/

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
/*DynamicTexture *dyntex;
int frame = 0;


//----------------------------------------------------------------------------------------------------------------------
void UMyBlueprintFunctionLibrary::KuTextureUpdate(UObject * WorldContextObject) {
	frame++;
	//frame %= 255;
	int w = dyntex->Width;
	int h = dyntex->Height;
	DynamicTexturePixel* pixels = dyntex->Pixels;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			DynamicTexturePixel &pix = pixels[x + w*y];
			pix.r = (x + frame) % w;
			pix.g = 128;
			pix.b = ((y/20) % 2 == 0)? 255:0;
			pix.a = 255;
		}

	}
	dyntex->Update();

}

//----------------------------------------------------------------------------------------------------------------------
void UMyBlueprintFunctionLibrary::KuTextureInit(UObject * WorldContextObject) {

	dyntex = new DynamicTexture();

	UWorld * World = GEngine->GetWorldFromContextObject(WorldContextObject);

	//AActor
	for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
	{

		string name = //TCHAR_TO_UTF8(*ActorItr->GetName());
			TCHAR_TO_UTF8(*UKismetSystemLibrary::GetDisplayName(*ActorItr));
		AStaticMeshActor *Actor = *ActorItr;

		if (name == "Plane_Blueprint") {
			KU_PRINT(name);
			KU_PRINT("Found");


			TArray<UStaticMeshComponent*> Components;
			Actor->GetComponents<UStaticMeshComponent>(Components);
			for (int32 i = 0; i<Components.Num(); i++)
			{
				UStaticMeshComponent* StaticMeshComponent = Components[i];
				UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh;
				KU_PRINT("get StaticMesh");

				int32 materialIndex = 0;
				int w = 256;
				int h = 256;

				KU_LOG("Initialize...");
				dyntex->Initialize(StaticMeshComponent, materialIndex, w, h);

				KU_LOG("Update...");


				dyntex->Update();

				KU_PRINT("Updated");
				return; 

			}

			//KuActor actor;
			//actor.setup(name, Mesh);
			//ku_actors_.push_back(actor);
			//UE_LOG(LogTemp, Warning, TEXT(name.c_str())); //TEXT("Your message"));
		}

	}





	//----------

	/*texture1 = UTexture2D::CreateTransient(256, 256, PF_B8G8R8A8);
	texture1->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	texture1->SRGB = 0;
	//Guarantee no garbage collection by adding it as a root reference
	texture1->AddToRoot();
	//Update the texture with new variable values.
	texture1->UpdateResource();
	

	//---------
	UMaterialInstanceDynamic* RV_MatInst = UMaterialInstanceDynamic::Create(MasterMaterialRef, this);
	RV_MatInst->SetTextureParameterValue(FName("T2DParam"), texture1);
	//---------
	*/

	/*
	//Grab the colorvalues from our existing texture (the one we created at '''Texture Setup''') and copy it into a uint8* mTextureColors variable.
	int32 w, h;
	w = textureToReadFrom->GetSizeX();
	h = textureToReadFrom->GetSizeY();
	FTexture2DMipMap& readMip = textureToReadFrom->PlatformData->Mips[0];
	mDataSize = w * h * 4; // * 4 because we're working with uint8's - which are 4 bytes large
	mDataSqrtSize = w * 4; // * 4 because we're working with uint8's - which are 4 bytes large
	readMip.BulkData.GetCopy((void**)&mTextureColors);
	// Initalize our dynamic pixel array with data size
	mDynamicColors = new uint8[mDataSize];
	// Copy our current texture's colors into our dynamic colors
	FMemory::Memcpy(mDynamicColors, mTextureColors, mDataSize);
	// Create a new texture region with the width and height of our dynamic texture
	mUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, w, h);
	// Set the Paramater in our material to our texture
	mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);
	*/
	/*
	int32 w = 256;
	int32 h = w;

	mDynamicMaterials.Empty();
	mDynamicMaterials.Add(GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0));
	mDynamicTexture = UTexture2D::CreateTransient(w, h);
	mDynamicTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	mDynamicTexture->SRGB = 0;
	mDynamicTexture->AddToRoot();
	mDynamicTexture->UpdateResource();

	mUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, w, h);

	mDynamicMaterials[0]->SetTextureParameterValue("DynamicTextureParam", mDynamicTexture);
	mDataSize = w * h * 4;
	mDataSqrtSize = w * 4;
	mArraySize = w * h;
	mRowSize = w;
	mDynamicColors = new uint8[mDataSize];
	mDynamicColorsFloat = new float[mArraySize];
	*/
	
	//mArrayOfIndexes.Empty();
	//FMemory::Memcpy(mDynamicColors, ExportedLavaValues->GetData(), mDataSize);
 
	//for (int i = 0; i < mArraySize; ++i)
	//{
	//	mDynamicColorsFloat[i] = static_cast<float>(mDynamicColors[i * 4 + RED]);
	//	if (mDynamicColors[i * 4 + RED] > mDynamicColors[i * 4 + BLUE])
	//		mArrayOfIndexes.Add(i);
	//}
	
	

//}

//----------------------------------------------------------------------------------------------------------------------


