#pragma once

//Manipulating with Actor

#include "Engine.h"
#include "KuUtils.h"


struct KuActor {
	KuActor() {
		active_pos = false;
		active_rot = false;
		actor = 0;
	}
	void setup(std::string name0, AActor *actor0) {
		name = name0;
		actor = actor0;
	}
	void set_pos(float x, float y, float z, float smooth) {
		target_pos.X = x;
		target_pos.Y = y;
		target_pos.Z = z;		
		smooth_pos = smooth;
		active_pos = true;
	}
	void set_rot(float yaw, float pitch, float roll, float smooth) {
		target_rot.Yaw = yaw;
		target_rot.Pitch = pitch;
		target_rot.Roll = roll;		
		smooth_rot = smooth;
		active_rot = true;
	}
	void set_posrot(float x, float y, float z, float yaw, float pitch, float roll, float smooth_pos0, float smooth_rot0) {
		set_pos(x, y, z, smooth_pos0);
		set_rot(yaw, pitch, roll, smooth_rot0);
	}

	void update() {
		if (active_pos && actor) {
			FVector v = actor->GetActorLocation();		//To object be movable - set "Movable" in its properties
			v += (target_pos - v)*(1 - smooth_pos);
			bool res = actor->SetActorLocation(v, false);
			//if (!res) KU_PRINT("Can't set location for " + name);			
		}
		if (active_rot && actor) {
			FRotator rot = actor->GetActorRotation();	//TODO warp angles!
			rot += (target_rot - rot)*(1 - smooth_rot);
			bool res = actor->SetActorRotation(rot);
			//if (!res) KU_PRINT("Can't set rotation for " + name);			
		}
	}

	std::string name;
	AActor *actor;

	bool active_pos, active_rot;

	FVector target_pos;		//target position
	FRotator target_rot;	//target rotation
	float smooth_pos;
	float smooth_rot;


};
