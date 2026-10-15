#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

void AFDWorld::RunFourViewTests(float DeltaSeconds)
{
    SmokeClock+=DeltaSeconds;
    const int32 View=SmokeStep/9, Stage=SmokeStep%9;
    const FVector R=FDProjection::Right(View), N=FDProjection::Depth(View);
    const FVector Base=RoomOrigin(4)+FVector(2200,2400,1600);
    const FString Prefix=FString::Printf(TEXT("View %d: "),View);
    const auto Assert=[&](bool OK,const TCHAR* Label) { Check(OK,Prefix+Label); };
    const auto Remove=[&](const TCHAR* Id)
    { if (AFDObject* O=Find(Id)) { Objects.Remove(O); O->Destroy(); } };
    if (Stage==2 && SmokeClock>.35f && SmokeClock<.65f) Player->MoveRight(1);
    if (SmokeClock<(Stage==2 ? .8f : (Stage==7 ? 2.f : .7f))) return;
    SmokeClock=0;
    switch(Stage)
    {
    case 0:
        Player->ApplyDimension(false,Base+FVector(0,0,118));
        Player->FoldView=View;
        Add(EFDKind::Platform,4,TEXT("four_source"),Base-RoomOrigin(4),FVector(600,280,60),FLinearColor::Gray);
        Add(EFDKind::Platform,4,TEXT("four_target"),Base-RoomOrigin(4)+R*900+N*850,FVector(600,280,60),FLinearColor::Gray);
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        break;
    case 1:
        Assert(Player->TrySwitch(),TEXT("grounded fold succeeds"));
        Assert(FVector::DotProduct(Player->GetCharacterMovement()->GetPlaneConstraintNormal(),N)>.99f,TEXT("movement plane matches selected depth axis"));
        {
            AFDObject* Target=Find(TEXT("four_target"));
            Assert(FMath::IsNearlyEqual(FDProjection::Width(Target->Projection->GetUnscaledBoxExtent(),View),FDProjection::Width(Target->Size,View)*.5f),TEXT("asymmetric platform uses the correct projected width"));
            const int32 Before=Player->FoldView;
            Assert(!Player->SelectView(View+1) && Player->FoldView==Before,TEXT("in-plane direction change requires unfolding first"));
        }
        break;
    case 2:
        // The spring-arm socket updates on its component tick after the switch.
        UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_CAMERA view=%d selected=%d camera=%s boom=%s actor=%s absolute=%d control=%d"),View,Player->FoldView,*Player->Camera->GetComponentRotation().ToString(),*Player->Boom->GetComponentRotation().ToString(),*Player->GetActorRotation().ToString(),Player->Boom->IsUsingAbsoluteRotation(),Player->Boom->bUsePawnControlRotation);
        Assert(FVector::DotProduct(Player->Camera->GetForwardVector(),-N)>.99f,TEXT("camera looks from the selected cardinal side after its update"));
        Assert(FVector::DotProduct(Player->GetActorLocation()-Base,R)>10,TEXT("D physically moves toward screen-right, including mirrored views"));
        Assert(FMath::Abs(FVector::DotProduct(Player->GetActorLocation(),N)-Player->PlaneDepth)<1,TEXT("movement preserves collapsed depth"));
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->SetActorLocation(FDProjection::OnPlane(Find(TEXT("four_target"))->GetActorLocation()+FVector(0,0,118),View,Player->PlaneDepth));
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        break;
    case 3:
        {
            const FVector Before=Player->GetActorLocation();
            const FVector Landing=Find(TEXT("four_target"))->GetActorLocation()+FVector(0,0,118);
            Add(EFDKind::Platform,4,TEXT("four_block"),Landing-RoomOrigin(4),FVector(70,70,130),FLinearColor::Gray);
            Assert(!Player->TrySwitch() && Player->b2D && Player->GetActorLocation().Equals(Before),TEXT("occupied source rejects unfolding without changing state"));
            Remove(TEXT("four_block"));
            Assert(Player->TrySwitch(),TEXT("safe source unfolds"));
            Assert(FMath::Abs(FVector::DotProduct(Player->GetActorLocation()-Landing,N))<1,TEXT("unfold restores the support's true X or Y depth"));
        }
        break;
    case 4:
        {
            const FVector Target=Find(TEXT("four_target"))->GetActorLocation();
            Add(EFDKind::Vine,4,TEXT("four_vine"),Target-RoomOrigin(4)+R*230+N*500+FVector(0,0,210),FVector(70,70,360),FLinearColor::Yellow);
            Assert(Player->TrySwitch(),TEXT("fold for a cross-depth fire target succeeds"));
            Player->SetActorRotation(R.Rotation());
        }
        break;
    case 5: PlaceFire(); break;
    case 6:
        {
            AFDObject* Rune=Add(EFDKind::Receiver,4,TEXT("four_rune"),FireSourceAnchor-RoomOrigin(4)+FVector(0,0,100),FVector(60),FLinearColor::Yellow);
            Rune->RequiredView=(View+1)%4;
        }
        break;
    case 7:
        Assert(Find(TEXT("four_vine"))->bBurned,TEXT("fire wall burns a target across the selected depth axis"));
        Assert(Find(TEXT("four_vine"))->Projection->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("burn removes this view's projected collision"));
        Assert(!Find(TEXT("four_rune"))->bActivated,TEXT("wrong-view fire cannot activate a directional seal"));
        Find(TEXT("four_rune"))->RequiredView=View;
        PlaceFire();
        break;
    case 8:
        {
            Assert(Find(TEXT("four_rune"))->bActivated,TEXT("matching view activates a directional seal"));
            const FVector Source=FireSourceAnchor; const float Lifetime=FireRemaining;
            Assert(Player->TrySwitch(),TEXT("unfold with an active spell succeeds"));
            Assert(FireSourceAnchor.Equals(Source) && FireAnchor.Equals(Source) && FireRemaining<=Lifetime,TEXT("unfold preserves the spell source and remaining lifetime"));
            Remove(TEXT("four_source")); Remove(TEXT("four_target")); Remove(TEXT("four_vine")); Remove(TEXT("four_rune"));
            FireRemaining=0;
            if (View==3)
            {
                EnterRoom(4,false);
                Assert(!Find(TEXT("r4_seal0"))->bActivated && !Find(TEXT("r4_seal1"))->bActivated && !Find(TEXT("r4_seal2"))->bActivated && !Find(TEXT("r4_seal3"))->bActivated,TEXT("room reset clears all directional seals"));
                UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_TEST COMPLETE failures=%d"),SmokeFailures);
                FPlatformMisc::RequestExitWithStatus(false,SmokeFailures ? 1 : 0);
            }
        }
        break;
    }
    ++SmokeStep;
}
