#include "FDVisuals.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

void FDVisuals::BuildScenery(AActor* Owner,USceneComponent* Parent,int32 Room)
{
    // One instanced component per mesh/material combination. Hidden room roots
    // hide their full hierarchy; ornaments never acquire gameplay collision.
    TMap<FString,UInstancedStaticMeshComponent*> Batches;
    const auto Imported=[&](const TCHAR* Name,FVector Position,FVector Scale,FRotator Rotation=FRotator::ZeroRotator)
    {
        const FString Key=FString(TEXT("KayKit_"))+Name;
        UInstancedStaticMeshComponent* Batch=Batches.FindRef(Key);
        if (!Batch)
        {
            const FString Path=FString::Printf(TEXT("/Game/Foldlight/External/KayKit/Environment/SM_KK_%s.SM_KK_%s"),Name,Name);
            UStaticMesh* Mesh=LoadObject<UStaticMesh>(nullptr,*Path);
            if (!Mesh) return;
            Batch=NewObject<UInstancedStaticMeshComponent>(Owner);
            Owner->AddInstanceComponent(Batch); Batch->SetupAttachment(Parent);
            Batch->SetMobility(EComponentMobility::Movable);
            Batch->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Batch->SetStaticMesh(Mesh); Batch->SetCastShadow(false);
            Batch->RegisterComponent(); Batches.Add(Key,Batch);
        }
        Batch->AddInstance(FTransform(Rotation,Position,Scale));
    };
    const auto Part=[&](const TCHAR* Shape,FVector P,FVector S,FLinearColor C,float Glow=0.f,FRotator Rot=FRotator::ZeroRotator)
    {
        const FString Key=FString(Shape)+C.ToString()+FString::SanitizeFloat(Glow);
        UInstancedStaticMeshComponent* Batch=Batches.FindRef(Key);
        if (!Batch)
        {
            Batch=NewObject<UInstancedStaticMeshComponent>(Owner);
            Owner->AddInstanceComponent(Batch);Batch->SetupAttachment(Parent);
            Batch->SetMobility(EComponentMobility::Movable);
            Batch->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            const bool Custom=FString(Shape).StartsWith(TEXT("SM_"));
            const FString MeshPath=Custom ? FString::Printf(TEXT("/Game/Foldlight/Art/Models/%s.%s"),Shape,Shape) : FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape);
            Batch->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*MeshPath));
            UMaterialInstanceDynamic* Mat=Material(Owner,C,Glow);
            if (FString(Shape)==TEXT("SM_Stone") || FString(Shape).Contains(TEXT("Arch"))) Mat->SetScalarParameterValue(TEXT("Grain"),1.f);
            if (FString(Shape)==TEXT("SM_Root") || FString(Shape)==TEXT("SM_Leaf") || FString(Shape)==TEXT("SM_Grass") || FString(Shape)==TEXT("SM_Banner"))
            { Mat->SetScalarParameterValue(TEXT("Metallic"),0); Mat->SetScalarParameterValue(TEXT("Roughness"),.95f); }
            Batch->SetMaterial(0,Mat);Batch->SetCastShadow(false);
            Batch->RegisterComponent();Batches.Add(Key,Batch);
        }
        Batch->AddInstance(FTransform(Rot,P,S/100.f));
    };
    const FLinearColor Palettes[]={FLinearColor(.16f,.28f,.30f),FLinearColor(.20f,.24f,.32f),FLinearColor(.18f,.29f,.23f),FLinearColor(.29f,.20f,.18f),FLinearColor(.20f,.24f,.34f)};
    const FLinearColor Base=Palettes[FMath::Clamp(Room,0,4)];
    const FLinearColor Accent=Room==3 ? FLinearColor(.76f,.28f,.10f) : FLinearColor(.26f,.55f,.55f);
    // Atmospheric rear wall is behind both layers of the open arcade.
    Part(TEXT("Cube"),FVector(0,-2100,200),FVector(10000,90,4200),Base*.40f);
    for (int I=-5;I<=5;++I)
    {
        const float X=I*650.f;
        Part(TEXT("SM_Arch"),FVector(X,-1720,770),FVector(650,85,650),Base*.65f);
        for (float DX : {-290.f,290.f})
            Part(TEXT("SM_Stone"),FVector(X+DX,-1710,40),FVector(56,90,1130),Base*.65f);
    }
    for (int I=-5;I<=5;++I)
    {
        const float X=I*650.f;
        // An arch model occupies half its nominal height; its spring line is
        // center minus one quarter of that height.
        const bool Broken=(Room==1 || Room==2) && (FMath::Abs(I)%3==1);
        Part(Broken ? TEXT("SM_BrokenArch") : TEXT("SM_Arch"),FVector(X,-1330,635),FVector(650,130,650),Base*1.15f);
        for (float DX : {-325.f})
        {
            Imported(TEXT("pillar_decorated"),FVector(X+DX,-1370,-485),FVector(1.2f));
            Imported(TEXT("pillar_decorated"),FVector(X+DX,-1370,-5),FVector(1.2f));
        }
        // Suspended brass lantern and a cool crystal behind each opening.
        Part(TEXT("Cylinder"),FVector(X,-1380,660),FVector(3,3,230),Gold*.6f);
        Part(TEXT("SM_Ring"),FVector(X,-1380,525),FVector(90,90,5),Gold,.25f);
        Part(TEXT("SM_Crystal"),FVector(X,-1380,540),FVector(32,32,82),Accent,1.2f);
        Part(TEXT("SM_Pedestal"),FVector(X,-1430,-240),FVector(110,110,70),Base);
        Part(TEXT("SM_Crystal"),FVector(X,-1430,-145),FVector(65,65,145),Accent,.3f,FRotator(0,0,12));
        if ((Room==0 || Room==4) && I%2==0)
        {
            const FLinearColor Fabric=Room==0 ? FLinearColor(.08f,.24f,.25f) : FLinearColor(.22f,.13f,.31f);
            Part(TEXT("SM_Banner"),FVector(X+190,-1430,250),FVector(140,100,365),Fabric);
            Part(TEXT("Cylinder"),FVector(X+190,-1420,435),FVector(7,7,170),Gold*.7f,0,FRotator(90,0,0));
            Part(TEXT("SM_Ring"),FVector(X+190,-1400,295),FVector(64,64,3),Gold*.8f,.15f,FRotator(0,0,90));
            Part(TEXT("SM_Crystal"),FVector(X+190,-1395,295),FVector(18,4,43),Gold*.8f,.15f);
        }
    }
    // Author-made masonry and small vignettes occupy the rear arcade only.
    // Its root follows the active projection, keeping all four silhouettes clear.
    for (int I=-4;I<=4;++I)
    {
        const float X=I*650.f;
        if (I%2==0)
            Imported(Room==2 ? TEXT("wall_broken") : Room==4 ? TEXT("wall_archedwindow_open") : TEXT("wall_arched"),FVector(X,-1650,-485),FVector(1.6f));
        Imported(TEXT("torch_mounted"),FVector(X+230,-1210,185),FVector(1.5f));
        if (I%2!=0)
        {
            Part(TEXT("SM_Stone"),FVector(X,-1280,-325),FVector(410,250,60),Base*1.2f);
            if (Room==0 || Room==4)
            {
                Imported(TEXT("table_small_decorated_A"),FVector(X-85,-1260,-295),FVector(1.55f));
                Imported(TEXT("candle_triple"),FVector(X+115,-1260,-295),FVector(1.35f));
                Imported(TEXT("shelf_small_candles"),FVector(X,-1450,150),FVector(1.7f));
            }
            else if (Room==1 || Room==3)
            {
                Imported(TEXT("barrel_large_decorated"),FVector(X-75,-1310,-295),FVector(1.1f));
                Imported(TEXT("barrel_small_stack"),FVector(X+100,-1250,-295),FVector(.9f));
                Imported(TEXT("shelves"),FVector(X,-1460,-10),FVector(1.25f));
            }
            else Imported(TEXT("rubble_large"),FVector(X,-1260,-295),FVector(.48f,.65f,.6f));
        }
    }
    // Broken cornice and inset masonry courses; no full dark wall in front of it.
    for (int I=-8;I<=8;++I)
    {
        Part(TEXT("SM_Stone"),FVector(I*405.f,-1350,845),FVector(397,200,54),Base*1.35f);
        Part(TEXT("SM_Stone"),FVector(I*405.f,-1360,-540),FVector(397,210,74),Base);
        Part(TEXT("SM_Stone"),FVector(I*405.f,-1440,-615),FVector(394,280,74),Base*.8f);
    }
    // Monumental, distant astrolabe provides an identifiable room landmark.
    const FVector Center(300,-1940,490);
    if (Room==0 || Room==4)
    {
    Part(TEXT("SM_Ring"),Center,FVector(900,900,15),Gold*.65f,.12f,FRotator(0,0,90));
    Part(TEXT("SM_Ring"),Center+FVector(0,8,0),FVector(750,750,9),Accent,.35f,FRotator(0,0,90));
    for (int I=0;I<16;++I)
    {
        const float A=I*PI/8;
        Part(TEXT("SM_Crystal"),Center+FVector(FMath::Cos(A)*414,20,FMath::Sin(A)*414),FVector(15,14,36),Gold*.8f,.25f,FRotator(0,0,FMath::RadiansToDegrees(A)-90));
    }
    }
    if (Room==1)
    {
        for (int I=-2;I<=2;++I)
        {
            const float Diameter=I%2==0 ? 580.f : 340.f;
            Part(TEXT("SM_Gear"),FVector(I*390,-1880,330+(I%2)*230),FVector(Diameter,Diameter,26),Gold*.45f,.10f,FRotator(0,0,90));
            Part(TEXT("SM_Ring"),FVector(I*390,-1850,330+(I%2)*230),FVector(Diameter*.45f,Diameter*.45f,7),Accent,.45f,FRotator(0,0,90));
        }
    }
    if (Room==3)
    {
        for (int I=-3;I<=3;++I)
        {
            const FVector P(I*800,-1740,110);
            Part(TEXT("SM_Arch"),P+FVector(0,0,180),FVector(440,80,540),Base*1.7f);
            Part(TEXT("SM_Flame"),P+FVector(0,-90,40),FVector(90,80,245),FLinearColor(.75f,.15f,.025f),.9f);
            for (int J=-2;J<=2;++J)
                Part(TEXT("SM_Stone"),P+FVector(J*57,25,-50),FVector(13,23,380),Base*.55f);
            Part(TEXT("SM_Stone"),P+FVector(0,30,-235),FVector(440,140,45),Base);
        }
    }
    // Seeded low-poly rubble and firefly motes, shared in instanced batches.
    FRandomStream Random(410+Room);
    for (int I=0;I<48;++I)
    {
        const float X=Random.FRandRange(-3100,3100), Z=Random.FRandRange(-1000,-660);
        Part(TEXT("SM_Stone"),FVector(X,Random.FRandRange(-1700,-1000),Z),FVector(Random.FRandRange(45,150),90,Random.FRandRange(35,110)),Base*.8f,0,FRotator(Random.FRandRange(-30,30),Random.FRandRange(0,180),Random.FRandRange(-30,30)));
        Part(TEXT("Sphere"),FVector(Random.FRandRange(-3000,3000),Random.FRandRange(-1900,-1050),Random.FRandRange(-300,1300)),FVector(Random.FRandRange(2,5)),Accent,.9f);
    }
    if (Room==2 || Room==3)
    {
        for (int I=-4;I<=4;++I)
        {
            Part(TEXT("SM_Root"),FVector(I*620.f,-1220,220),FVector(390,220,1050),Room==2 ? FLinearColor(.15f,.24f,.14f) : FLinearColor(.23f,.12f,.08f),0,FRotator(0,0,(I%2)*12));
            for (int J=0;J<3;++J)
                Part(TEXT("SM_Crystal"),FVector(I*620.f+45,-1210,100+J*120),FVector(65,28,32),Accent,.25f,FRotator(0,0,J*45));
            if (Room==2)
            {
                for (int J=0;J<7;++J)
                {
                    const float Side=J%2 ? -1.f : 1.f;
                    Part(TEXT("SM_Leaf"),FVector(I*620.f+Side*56,-1190,-100+J*100),FVector(94,160,150),FLinearColor(.13f,.31f,.19f),0,FRotator(0,0,Side*55));
                    Part(TEXT("SM_Grass"),FVector(I*620.f+J*26-80,-1280,-490),FVector(120,120,130),FLinearColor(.12f,.25f,.14f));
                }
            }
        }
    }
    if (Room==4)
    {
        const FVector Stars[]={FVector(-1280,-1850,410),FVector(-960,-1850,720),FVector(-480,-1850,575),FVector(-190,-1850,955),FVector(380,-1850,1080),FVector(890,-1850,720),FVector(1280,-1850,840)};
        for (int I=0;I<7;++I)
        {
            Part(TEXT("SM_Crystal"),Stars[I],FVector(15,10,24),Accent,1.4f);
            if (I<6)
            {
                const FVector D=Stars[I+1]-Stars[I];
                Part(TEXT("Cube"),(Stars[I]+Stars[I+1])*.5f,FVector(D.Size(),2,2),Accent,.3f,D.Rotation());
            }
        }
    }
}
