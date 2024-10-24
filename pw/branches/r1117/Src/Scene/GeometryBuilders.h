#pragma once
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace NScene
{
class CollisionGeometry;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CollisionGeometry* CreatePyramid( const CVec3& center, const CVec3& size, float topSideSize, bool twoSided
																 , vector<CVec3>& points, vector<CollisionGeometry::Triangle>& tris );
CollisionGeometry* CreatePyramid( const SBound& bound, float topSideSize, bool twoSided, 
																 vector<CVec3>& points, vector<CollisionGeometry::Triangle>& triangles );
CollisionGeometry* CreateSphere( float radius, int subs );
CollisionGeometry* CreateCylinder( float radius, int subs, float height );
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
