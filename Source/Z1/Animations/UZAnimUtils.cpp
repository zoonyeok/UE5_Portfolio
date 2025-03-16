// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/UZAnimUtils.h"
#include "Engine/OverlapResult.h"

FTransform UUZAnimUtils::GetBoneTransformFromAnim(UAnimSequence* AnimSequence, FName BoneName, float Time)
{
	if (!AnimSequence)
	{
		return FTransform::Identity;
	}

	IAnimationDataModel* AnimationDataModel = AnimSequence->GetDataModel();
	if (!AnimationDataModel)
	{
		return FTransform::Identity;
	}
	const TArray<FBoneAnimationTrack>& Tracks = AnimationDataModel->GetBoneAnimationTracks();
	const FReferenceSkeleton& RefSkeleton = AnimSequence->GetSkeleton()->GetReferenceSkeleton();

	int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE)
	{
		return FTransform::Identity;
	}

	for (const FBoneAnimationTrack& Track : Tracks)
	{
		if (Track.BoneTreeIndex == BoneIndex)
		{
			const FRawAnimSequenceTrack& RawTrack = Track.InternalTrackData;

			// 현재 시간(Time)에서 키프레임 인덱스 계산
			int32 KeyframeIndex = FMath::FloorToInt((Time / AnimSequence->GetPlayLength()) * RawTrack.PosKeys.Num());
			KeyframeIndex = FMath::Clamp(KeyframeIndex, 0, RawTrack.PosKeys.Num() - 1);

			FVector Position = FVector(RawTrack.PosKeys[KeyframeIndex]);
			FQuat Rotation = FQuat(RawTrack.RotKeys[KeyframeIndex]);
			FVector Scale = FVector(RawTrack.ScaleKeys[KeyframeIndex]);

			return FTransform(Rotation, Position, Scale);
		}
	}

	return FTransform::Identity;
}

FTransform UUZAnimUtils::LerpTransform(const FTransform& A, const FTransform& B, float Alpha)
{
	FTransform Result;
	// 위치 보간
	Result.SetTranslation(FMath::Lerp(A.GetTranslation(), B.GetTranslation(), Alpha));
	// 스케일 보간
	Result.SetScale3D(FMath::Lerp(A.GetScale3D(), B.GetScale3D(), Alpha));
	// 회전 보간(Slerp)
	FQuat RotA = A.GetRotation();
	FQuat RotB = B.GetRotation();
	FQuat Rot = FQuat::Slerp(RotA, RotB, Alpha);
	Rot.Normalize();
	Result.SetRotation(Rot);
	return Result;
}