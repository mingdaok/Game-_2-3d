#pragma once
#include "CoreMinimal.h"

// Cardinal camera positions; screen-right and collapsed depth share this basis.
namespace FDProjection
{
    inline int32 Wrap(int32 View) { return (View%4+4)%4; }
    inline FVector Depth(int32 View)
    {
        static const FVector Axes[]={FVector(0,1,0),FVector(1,0,0),FVector(0,-1,0),FVector(-1,0,0)};
        return Axes[Wrap(View)];
    }
    inline FVector Right(int32 View) { const FVector N=Depth(View); return FVector(N.Y,-N.X,0); }
    inline float Yaw(int32 View) { return (-Depth(View)).Rotation().Yaw; }
    inline float Width(FVector Size,int32 View) { return FVector::DotProduct(Size,Right(View).GetAbs()); }
    inline FVector OnPlane(FVector Point,int32 View,float DepthValue)
    { const FVector N=Depth(View); return Point+N*(DepthValue-FVector::DotProduct(Point,N)); }
    inline const TCHAR* Name(int32 View)
    { static const TCHAR* Names[]={TEXT("南视"),TEXT("东视"),TEXT("北视"),TEXT("西视")}; return Names[Wrap(View)]; }
}
