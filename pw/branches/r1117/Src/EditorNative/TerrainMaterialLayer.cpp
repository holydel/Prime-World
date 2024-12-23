#include "stdafx.h"
#include "Utility.h"
#include "EditorTerrain.h"
#include "TerrainMaterialLayer.h"

using namespace System;

using namespace EditorNative;
using namespace EditorNative::Terrain;
using namespace EditorNative::Terrain::Layers;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TerrainMaterialLayer::TerrainMaterialLayer( )
:	index( -1 )
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TerrainMaterialLayer::TerrainMaterialLayer( int _index )
:	index( _index )
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Byte TerrainMaterialLayer::default::get( int row, int column )
{
	return IsValid ? Owner->Native->ReadLayerValue( index, row, column ) : 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TerrainMaterialLayer::default::set( int row, int column, Byte value )
{
	if ( IsValid && this[row, column] != value )
	{
		Owner->Native->WriteLayerValue( index, row, column, value );
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
