#include "FDMageAnimInstance.h"
#include "FoldlightDemo.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AnimSequence.h"
#include "AnimNodes/AnimNode_TwoWayBlend.h"
#include "AnimNodes/AnimNode_LayeredBoneBlend.h"
#include "GameFramework/CharacterMovementComponent.h"

class FFDMageProxy final : public FAnimInstanceProxy
{
public:
    explicit FFDMageProxy(UAnimInstance* Owner) : FAnimInstanceProxy(Owner) {}
    FAnimNode_SequencePlayer_Standalone Idle, Walk, Run, JumpStart, JumpLoop, Land, Cast;
    FAnimNode_TwoWayBlend Gait, Moving, Jump, Air, Landing;
    FAnimNode_LayeredBoneBlend UpperBody;
    float Speed=0, AirTime=0, LandTime=0, CastTime=0;
    bool bAir=false;
    int32 LastCast=0;

    virtual void Initialize(UAnimInstance* InInstance) override
    {
        auto* Instance=CastChecked<UFDMageAnimInstance>(InInstance);
        Instance->Clips.Reset();
        const TCHAR* Names[]={TEXT("Idle"),TEXT("Walking_A"),TEXT("Running_A"),TEXT("Jump_Start"),TEXT("Jump_Idle"),TEXT("Jump_Land"),TEXT("Spellcast_Shoot")};
        FAnimNode_SequencePlayer_Standalone* Players[]={&Idle,&Walk,&Run,&JumpStart,&JumpLoop,&Land,&Cast};
        for (int32 I=0;I<UE_ARRAY_COUNT(Names);++I)
        {
            const FString Path=FString::Printf(TEXT("/Game/Foldlight/External/KayKit/Mage/Mage/SkeletalMeshes/Mage%s.Mage%s"),Names[I],Names[I]);
            UAnimSequence* Clip=LoadObject<UAnimSequence>(nullptr,*Path);
            Instance->Clips.Add(Clip); Players[I]->SetSequence(Clip);
            Players[I]->SetLoopAnimation(I<3 || I==4);
        }
        // Keep walk/run phases in sync as speed changes.
        Walk.SetGroupName(TEXT("Gait")); Run.SetGroupName(TEXT("Gait"));
        Walk.SetGroupMethod(EAnimSyncMethod::SyncGroup); Run.SetGroupMethod(EAnimSyncMethod::SyncGroup);
        JumpStart.SetPlayRate(2.5f); Land.SetPlayRate(2.f);
        Gait.A.SetLinkNode(&Walk); Gait.B.SetLinkNode(&Run);
        Moving.A.SetLinkNode(&Idle); Moving.B.SetLinkNode(&Gait);
        Jump.A.SetLinkNode(&JumpStart); Jump.B.SetLinkNode(&JumpLoop);
        Air.A.SetLinkNode(&Moving); Air.B.SetLinkNode(&Jump);
        Landing.A.SetLinkNode(&Air); Landing.B.SetLinkNode(&Land);
        UpperBody.BasePose.SetLinkNode(&Landing);
        UpperBody.AddPose(); UpperBody.BlendPoses[0].SetLinkNode(&Cast);
        FBranchFilter Filter; Filter.BoneName=TEXT("spine"); Filter.BlendDepth=1;
        UpperBody.LayerSetup[0].BranchFilters.Add(Filter);
        UpperBody.bMeshSpaceRotationBlend=true;
        UpperBody.BlendWeights[0]=0;
        FAnimInstanceProxy::Initialize(InInstance);
    }
    virtual FAnimNode_Base* GetCustomRootNode() override { return &UpperBody; }
    virtual void GetCustomNodes(TArray<FAnimNode_Base*>& Nodes) override
    { Nodes.Append({&Idle,&Walk,&Run,&JumpStart,&JumpLoop,&Land,&Cast,&Gait,&Moving,&Jump,&Air,&Landing,&UpperBody}); }
    virtual void PreUpdate(UAnimInstance* Instance,float Dt) override
    {
        FAnimInstanceProxy::PreUpdate(Instance,Dt);
        const AFDCharacter* Hero=::Cast<AFDCharacter>(Instance->TryGetPawnOwner());
        if (!Hero) return;
        Speed=Hero->GetVelocity().Size2D();
        const bool NewAir=Hero->GetCharacterMovement()->IsFalling();
        if (NewAir && !bAir) { AirTime=0; JumpStart.SetAccumulatedTime(0); }
        if (!NewAir && bAir) { LandTime=.32f; Land.SetAccumulatedTime(0); }
        bAir=NewAir;
        if (Hero->CastSerial!=LastCast)
        { LastCast=Hero->CastSerial; CastTime=.9f; Cast.SetAccumulatedTime(0); }
    }
    virtual void Update(float Dt) override
    {
        AirTime=bAir ? AirTime+Dt : 0;
        LandTime=FMath::Max(0.f,LandTime-Dt); CastTime=FMath::Max(0.f,CastTime-Dt);
        Gait.Alpha=FMath::Clamp((Speed-140.f)/230.f,0.f,1.f);
        Moving.Alpha=FMath::FInterpTo(Moving.Alpha,FMath::Clamp(Speed/100.f,0.f,1.f),Dt,12);
        Walk.SetPlayRate(FMath::Clamp(Speed/160.f,.7f,1.5f));
        Run.SetPlayRate(FMath::Clamp(Speed/420.f,.7f,1.15f));
        Jump.Alpha=FMath::Clamp((AirTime-.12f)/.10f,0.f,1.f);
        Air.Alpha=FMath::FInterpTo(Air.Alpha,bAir ? 1.f : 0.f,Dt,18);
        Landing.Alpha=FMath::FInterpTo(Landing.Alpha,LandTime>.12f && !bAir ? .75f : 0.f,Dt,20);
        const float CastTarget=FMath::Min((.9f-CastTime)/.08f,CastTime/.18f);
        UpperBody.BlendWeights[0]=FMath::Clamp(CastTarget,0.f,1.f);
    }
};

FAnimInstanceProxy* UFDMageAnimInstance::CreateAnimInstanceProxy() { return new FFDMageProxy(this); }
void UFDMageAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) { delete Proxy; }
