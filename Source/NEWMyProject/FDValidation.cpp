#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/GameViewportClient.h"
#include "HighResScreenshot.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/PlatformMisc.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "FDMageAnimInstance.h"

void AFDWorld::Check(bool Condition,const FString& Label)
{
    if (Condition) { UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_TEST PASS: %s"),*Label); }
    else { SmokeFailures++; UE_LOG(LogTemp,Error,TEXT("FOLDLIGHT_TEST FAIL: %s"),*Label); }
}

void AFDWorld::RunSmoke(float DeltaSeconds)
{
    static FTransform InitialChestPose;
    if (FParse::Param(FCommandLine::Get(),TEXT("FoldlightTraversal"))) { RunTraversal(DeltaSeconds); return; }
    if (bFourTest) { RunFourViewTests(DeltaSeconds); return; }
    SmokeClock+=DeltaSeconds;
    if (bCapture)
    {
        int32 CaptureRoom=0; FParse::Value(FCommandLine::Get(),TEXT("CaptureRoom="),CaptureRoom);
        FString CaptureAction; FParse::Value(FCommandLine::Get(),TEXT("CaptureAction="),CaptureAction);
        if (SmokeStep==3 && (CaptureAction==TEXT("Run") || CaptureAction==TEXT("Walk")))
            Player->AddMovementInput(FDProjection::Right(Player->FoldView),1);
        if (SmokeStep==0 && SmokeClock>2)
        {
            EnterRoom(FMath::Clamp(CaptureRoom,0,4),false);
            if (CaptureRoom==0) Player->SetActorLocation(FVector(-610,0,90));
            if (CaptureRoom==0 && !CaptureAction.IsEmpty()) Player->SetActorLocation(FVector(-900,0,90));
            if (CaptureRoom==3) Player->SetActorLocation(RoomOrigin(3)+FVector(0,0,90));
            if (FParse::Param(FCommandLine::Get(),TEXT("CaptureTitle"))) bStarted=false;
            SmokeStep++; SmokeClock=0;
        }
        else if (SmokeStep==1 && SmokeClock>3)
        {
            int32 View=0; FParse::Value(FCommandLine::Get(),TEXT("CaptureView="),View);
            Player->SelectView(View);
            if (CaptureRoom==4)
            {
                const TCHAR* Platforms[]={TEXT("r4_south"),TEXT("r4_goal"),TEXT("r4_north"),TEXT("r4_west")};
                AFDObject* Landing=Find(Platforms[FDProjection::Wrap(View)]);
                Player->SetActorLocation(Landing->GetActorLocation()+FVector(0,0,120));
                Player->SetActorRotation(FDProjection::Right(View).Rotation());
                Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
            }
            else if (FParse::Param(FCommandLine::Get(),TEXT("Capture2D"))) Player->TrySwitch();
            SmokeStep++; SmokeClock=0;
        }
        else if (SmokeStep==2 && SmokeClock>2)
        {
            if (CaptureRoom==4 && FParse::Param(FCommandLine::Get(),TEXT("Capture2D")) && !Player->b2D)
            { Player->TrySwitch(); SmokeClock=0; return; }
            if (CaptureRoom>=2) PlaceFire();
            if (CaptureAction==TEXT("Walk")) Player->GetCharacterMovement()->MaxWalkSpeed=150;
            if (CaptureAction==TEXT("Jump")) Player->LaunchCharacter(FVector(0,0,540),false,true);
            SmokeStep++; SmokeClock=0;
        }
        else if (SmokeStep==3 && SmokeClock>(CaptureAction.IsEmpty() ? .7f : .35f))
        {
            FString Name=FString::Printf(TEXT("Foldlight_room%d_view%d_%s.png"),CaptureRoom,Player->FoldView,Player->b2D ? TEXT("2D") : TEXT("3D"));
            if (!CaptureAction.IsEmpty()) Name=Name.Replace(TEXT(".png"),*FString::Printf(TEXT("_%s.png"),*CaptureAction));
            // Capture real UI states without modifying player saves.
            if (FParse::Param(FCommandLine::Get(),TEXT("CaptureTitle"))) Name=TEXT("Foldlight_title.png");
            if (FParse::Param(FCommandLine::Get(),TEXT("CaptureHint")))
            { HintLevel=2; RequestHint(); Name=TEXT("Foldlight_hint.png"); }
            if (FParse::Param(FCommandLine::Get(),TEXT("CapturePreview")))
            { Player->PreviewStart(); Name=TEXT("Foldlight_preview.png"); }
            if (FParse::Param(FCommandLine::Get(),TEXT("CapturePause")))
            { Player->PauseToggle(); Name=TEXT("Foldlight_pause.png"); }
            if (FParse::Param(FCommandLine::Get(),TEXT("CaptureEnd")))
            { bFinished=true; Name=TEXT("Foldlight_end.png"); }
            FScreenshotRequest::RequestScreenshot(FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Screenshots"),Name),true,false);
            SmokeStep++; SmokeClock=0;
        }
        else if (SmokeStep==4 && SmokeClock>2) FPlatformMisc::RequestExit(false);
        return;
    }
    if (SmokeClock<1.3f) return;
    SmokeClock=0;
    const auto At=[&](FVector Local)
    {
        Player->SetActorLocation(RoomOrigin(CurrentRoom)+Local,false,nullptr,ETeleportType::TeleportPhysics);
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        Player->SetActorRotation(FRotator(0,0,0));
    };
    switch(SmokeStep++)
    {
    case 0:
        {
            USkeletalMeshComponent* HeroMesh=Player->GetMesh();
            UFDMageAnimInstance* Animation=Cast<UFDMageAnimInstance>(HeroMesh->GetAnimInstance());
            Check(Player->bSkeletalHero && HeroMesh->GetSkeletalMeshAsset() && Animation,TEXT("KayKit skeletal mage and native animation graph are active"));
            bool ClipsValid=Animation && Animation->Clips.Num()==7;
            if (Animation) for (UAnimSequence* Clip : Animation->Clips)
                ClipsValid &= Clip && Clip->GetPlayLength()>0 && Clip->GetSkeleton()==HeroMesh->GetSkeletalMeshAsset()->GetSkeleton();
            Check(ClipsValid,TEXT("All seven runtime animation clips match the imported mage skeleton"));
            Check(HeroMesh->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("Skeletal character preserves capsule-only gameplay collision"));
            InitialChestPose=HeroMesh->GetBoneTransform(HeroMesh->GetBoneIndex(TEXT("chest")),FTransform::Identity);
            bool ExternalPresent=true;
            for (const TCHAR* Name : {TEXT("pillar_decorated"),TEXT("wall_arched"),TEXT("wall_broken"),TEXT("torch_mounted"),TEXT("shelf_small_candles"),TEXT("table_small_decorated_A"),TEXT("barrel_large_decorated"),TEXT("barrel_small_stack"),TEXT("candle_triple"),TEXT("rubble_large"),TEXT("wall_archedwindow_open"),TEXT("shelves")})
                ExternalPresent &= LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/Foldlight/External/KayKit/Environment/SM_KK_%s.SM_KK_%s"),Name,Name))!=nullptr;
            Check(ExternalPresent,TEXT("All twelve licensed environment meshes are cooked into the package"));
            bool ModelsPresent=true, VisualsSafe=true, SceneryInstanced=true;
            for (const TCHAR* Name : {TEXT("SM_Stone"),TEXT("SM_Arch"),TEXT("SM_Ring"),TEXT("SM_Pedestal"),TEXT("SM_Crystal"),TEXT("SM_Flame"),TEXT("SM_Wax"),TEXT("SM_Cape"),TEXT("SM_Root"),TEXT("SM_Leaf"),TEXT("SM_Grass"),TEXT("SM_Banner"),TEXT("SM_Gear"),TEXT("SM_BrokenArch"),TEXT("SM_Sigil")})
                ModelsPresent &= LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/Foldlight/Art/Models/%s.%s"),Name,Name))!=nullptr;
            for (AFDObject* Object : Objects)
                for (UStaticMeshComponent* Mesh : Object->Pieces)
                    VisualsSafe &= Mesh->GetStaticMesh()!=nullptr && Mesh->GetCollisionEnabled()==ECollisionEnabled::NoCollision;
            for (USceneComponent* Root : SceneryRoots)
            {
                bool HasInstances=false;
                TArray<USceneComponent*> ScenicParts; Root->GetChildrenComponents(true,ScenicParts);
                for (USceneComponent* Child : ScenicParts)
                    if (UInstancedStaticMeshComponent* Batch=Cast<UInstancedStaticMeshComponent>(Child))
                    { HasInstances |= Batch->GetInstanceCount()>0; VisualsSafe &= Batch->GetCollisionEnabled()==ECollisionEnabled::NoCollision && Batch->GetStaticMesh()!=nullptr; }
                SceneryInstanced &= HasInstances;
            }
            Check(ModelsPresent,TEXT("Fifteen authored art meshes are included in the playable package"));
            Check(VisualsSafe,TEXT("Art meshes exist and cannot obstruct gameplay collision"));
            Check(SceneryInstanced,TEXT("Every room batches repeated scenery into mesh instances"));
            Check(LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Foldlight/Art/M_FireFlow.M_FireFlow"))!=nullptr,TEXT("The animated flame material is included in the package"));
            bool SpellSafe=FireSigils.Num()==2 && FireGlyphs.Num()==12;
            for (UStaticMeshComponent* Wisp : FirePieces) SpellSafe &= Wisp->GetStaticMesh()!=nullptr && Wisp->GetCollisionEnabled()==ECollisionEnabled::NoCollision;
            Check(SpellSafe,TEXT("Animated spell ornaments do not add gameplay collision"));
        }
        Check(Objects.Num()>=25,TEXT("Five procedural rooms have their gameplay objects"));
        Check(Player->GetCharacterMovement()->IsMovingOnGround(),TEXT("Player spawns on a real floor"));
        Check(Player->TrySwitch(),TEXT("Grounded 3D to 2D transition succeeds"));
        Check(Find(TEXT("r0_b"))->Projection->GetCollisionEnabled()!=ECollisionEnabled::NoCollision,TEXT("Distant source creates a walkable projection"));
        break;
    case 1:
        Check(!InitialChestPose.Equals(Player->GetMesh()->GetBoneTransform(Player->GetMesh()->GetBoneIndex(TEXT("chest")),FTransform::Identity),.0001f),TEXT("Imported animation changes bone pose across frames"));
        At(FVector(100,Player->PlaneDepth,232)); break;
    case 2:
        {
            AFDObject* Obstruction=Add(EFDKind::Platform,0,TEXT("test_blocked_destination"),FVector(100,500,230),FVector(100,100,150),FLinearColor::Gray);
            Check(!Player->TrySwitch(),TEXT("An occupied 3D destination rejects unfolding"));
            Check(Player->b2D && FMath::Abs(Player->GetActorLocation().Y-Player->PlaneDepth)<1,TEXT("Rejected transition preserves player dimension and position"));
            Objects.Remove(Obstruction); Obstruction->Destroy();
        }
        Check(Player->TrySwitch(),TEXT("2D support resolves to a safe 3D source"));
        Check(FMath::Abs(Player->GetActorLocation().Y-500)<1,TEXT("Unfold uses source depth, not a global deepest candidate"));
        break;
    case 3:
        At(FVector(100,500,650));
        Check(!Player->TrySwitch(),TEXT("Airborne folding is rejected"));
        EnterRoom(1,false); break;
    case 4:
        Find(TEXT("r1_pillar"))->Slide();
        Check(!Player->TrySwitch(),TEXT("Moving pillar blocks dimension commit")); break;
    case 5: break;
    case 6:
        Check(Find(TEXT("r1_pillar"))->bMoved,TEXT("Rail pillar reaches its authored stop"));
        ResetCurrentRoom();
        Check(!Find(TEXT("r1_pillar"))->bMoved && Find(TEXT("r1_pillar"))->GetActorLocation().Equals(Find(TEXT("r1_pillar"))->InitialLocation),TEXT("Room reset restores pillar transform and state"));
        EnterRoom(2,false); At(FVector(-140,0,90)); break;
    case 7: PlaceFire(); break;
    case 8: break;
    case 9:
        Check(Find(TEXT("r2_vine"))->bBurned,TEXT("3D sigil burns a real target after its burn delay"));
        Check(Find(TEXT("r2_vine"))->Solid->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("Burned source collision is removed"));
        Project(true,0);
        Check(Find(TEXT("r2_vine"))->Projection->GetCollisionEnabled()==ECollisionEnabled::NoCollision,TEXT("Burned source cannot leave a ghost projection"));
        ResetCurrentRoom();
        Check(!Find(TEXT("r2_vine"))->bBurned && Find(TEXT("r2_vine"))->Solid->GetCollisionEnabled()!=ECollisionEnabled::NoCollision,TEXT("Reset reconstructs a burned obstacle"));
        EnterRoom(3,false); At(FVector(-730,0,90)); break;
    case 10: Player->TrySwitch(); break;
    case 11: PlaceFire(); break;
    case 12:
        Check(!bFurnaceSolved,TEXT("Narrow fire wall cannot activate separated furnaces"));
        { float Life=FireRemaining; Player->TrySwitch(); Check(FireRemaining<=Life,TEXT("Changing dimensions does not refresh spell lifetime")); }
        break;
    case 13: PlaceFire(); break;
    case 14:
        Check(bFurnaceSolved,TEXT("Single 3D sigil activates both low furnaces"));
        At(FVector(220,0,90)); break;
    case 15: PlaceFire(); break;
    case 16:
        Check(!Find(TEXT("r3_vine"))->bBurning,TEXT("Low 3D fire does not reach suspended roots"));
        Player->TrySwitch(); break;
    case 17: PlaceFire(); break;
    case 18: break;
    case 19:
        Check(Find(TEXT("r3_vine"))->bBurned,TEXT("2D fire hits the projected target at a different source depth"));
        Check(RoomSolved(),TEXT("Both furnace room conditions latch correctly"));
        EnterRoom(4,false);
        bFourTest=true; SmokeStep=0; SmokeClock=0; break;
    }
}

void AFDWorld::RunTraversal(float DeltaSeconds)
{
    SmokeClock+=DeltaSeconds;
    if (SessionTime>240 || Falls>0)
    {
        UE_LOG(LogTemp,Error,TEXT("FOLDLIGHT_TRAVERSAL FAIL room=%d stage=%d falls=%d location=%s"),CurrentRoom,SmokeStep,Falls,*Player->GetActorLocation().ToString());
        FPlatformMisc::RequestExitWithStatus(false,1); return;
    }
    if (!Player->InputAllowed()) return;
    const FVector Local=Player->GetActorLocation()-RoomOrigin(CurrentRoom);
    const auto Walk=[&](FVector2D Target)
    {
        FVector Direction(Target.X-Local.X,Target.Y-Local.Y,0);
        if (Direction.Size2D()<15) { Player->GetCharacterMovement()->StopMovementImmediately(); return true; }
        Player->AddMovementInput(Direction.GetSafeNormal(),1); return false;
    };
    const auto Advance=[&](int32 Next) { SmokeStep=Next; SmokeClock=0; };
    const auto Leave=[&]()
    {
        const int32 Before=CurrentRoom;
        Player->Interact();
        if (bFinished)
        {
            UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_TRAVERSAL COMPLETE: all five rooms traversed with movement and jump physics; falls=%d"),Falls);
            FPlatformMisc::RequestExitWithStatus(false,0);
        }
        else if (CurrentRoom!=Before)
        {
            UE_LOG(LogTemp,Display,TEXT("FOLDLIGHT_TRAVERSAL PASS: room %d completed with movement physics"),Before+1);
            Advance(0);
        }
    };
    if (SmokeStep==0 && SmokeClock>1)
    {
        if (CurrentRoom==0) { if (Player->TrySwitch()) Advance(10); }
        else if (CurrentRoom==1 || CurrentRoom==4) Advance(1);
        else if (CurrentRoom==2) Advance(20);
        else Advance(30);
    }
    else if (SmokeStep==1)
    {
        if (Walk(FVector2D(CurrentRoom==1 ? -930 : -1000,CurrentRoom==1 ? -280 : -320))) { Player->Interact(); Advance(2); }
    }
    else if (SmokeStep==2)
    {
        AFDObject* Pillar=Find(CurrentRoom==1 ? TEXT("r1_pillar") : TEXT("r4_pillar"));
        if (Pillar->bMoved && Walk(FVector2D(-1100,0))) { if (Player->TrySwitch()) Advance(10); }
    }
    else if (SmokeStep==10)
    {
        TArray<FString> Path;
        if (CurrentRoom==0) Path={TEXT("r0_start"),TEXT("r0_a"),TEXT("r0_b"),TEXT("r0_c"),TEXT("r0_goal")};
        if (CurrentRoom==1) Path={TEXT("r1_start"),TEXT("r1_a"),TEXT("r1_pillar"),TEXT("r1_c"),TEXT("r1_goal")};
        if (CurrentRoom==3) Path={TEXT("r3_start"),TEXT("r3_step"),TEXT("r3_goal")};
        if (CurrentRoom==4)
        {
            switch(Player->FoldView)
            {
            case 0: Path={TEXT("r4_start"),TEXT("r4_pillar"),TEXT("r4_b"),TEXT("r4_south")}; break;
            case 3: Path={TEXT("r4_south"),TEXT("r4_w1"),TEXT("r4_west")}; break;
            case 2: Path={TEXT("r4_west"),TEXT("r4_n1"),TEXT("r4_north")}; break;
            default: Path={TEXT("r4_north"),TEXT("r4_e1"),TEXT("r4_goal")}; break;
            }
        }
        const FVector Right=FDProjection::Right(Player->FoldView);
        const float Position=FVector::DotProduct(Player->GetActorLocation(),Right);
        AFDObject* Support=Cast<AFDObject>(Player->GetCharacterMovement()->CurrentFloor.HitResult.GetActor());
        const int32 Index=Support ? Path.IndexOfByKey(Support->ObjectId) : INDEX_NONE;
        const bool Ground=Player->GetCharacterMovement()->IsMovingOnGround();
        if (Ground && Index==Path.Num()-1 && Position>=FVector::DotProduct(Support->GetActorLocation(),Right))
        {
            Player->GetCharacterMovement()->StopMovementImmediately();
            if (CurrentRoom==4)
            {
                Player->SetActorRotation(Right.Rotation());
                if (!Find(FString::Printf(TEXT("r4_seal%d"),Player->FoldView))->bActivated)
                { Player->CastFire(); return; }
                if (Player->TrySwitch()) Advance(Player->FoldView==1 ? 11 : 40);
            }
            else if (Player->TrySwitch()) Advance(11);
            return;
        }
        Player->MoveRight(1);
        if (Ground && Index>=0 && Index<Path.Num()-1)
        {
            AFDObject* Next=Find(Path[Index+1]);
            float Edge=FMath::Min(FVector::DotProduct(Support->GetActorLocation(),Right)+FDProjection::Width(Support->Size,Player->FoldView)*.5f,FVector::DotProduct(Next->GetActorLocation(),Right)-FDProjection::Width(Next->Size,Player->FoldView)*.5f)-75;
            if (Position>=Edge) Player->JumpPressed();
        }
    }
    else if (SmokeStep==40 && SmokeClock>.6f)
    {
        const int32 NextView=Player->FoldView==0 ? 3 : (Player->FoldView==3 ? 2 : 1);
        Player->SelectView(NextView);
        if (Player->TrySwitch()) Advance(10);
    }
    else if (SmokeStep==11 && SmokeClock>.6f) Leave();
    else if (SmokeStep==20)
    {
        if (Walk(FVector2D(-140,0))) { Player->SetActorRotation(FRotator::ZeroRotator); Player->CastFire(); Advance(21); }
    }
    else if (SmokeStep==21 && Find(TEXT("r2_vine"))->bBurned) Advance(22);
    else if (SmokeStep==22) { if (Walk(FVector2D(1030,0))) Leave(); }
    else if (SmokeStep==30)
    {
        if (Walk(FVector2D(-730,0))) { Player->SetActorRotation(FRotator::ZeroRotator); Player->CastFire(); Advance(31); }
    }
    else if (SmokeStep==31 && bFurnaceSolved) Advance(32);
    else if (SmokeStep==32)
    {
        if (Walk(FVector2D(220,0)) && Player->TrySwitch()) Advance(33);
    }
    else if (SmokeStep==33)
    {
        Player->SetActorRotation(FRotator::ZeroRotator); Player->CastFire(); Advance(34);
    }
    else if (SmokeStep==34 && Find(TEXT("r3_vine"))->bBurned) Advance(10);
}
