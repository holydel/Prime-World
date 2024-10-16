#include "stdafx.h"
#include "Camera.h"
#include "EditorScene.h"
#include "EditorSound.h"

using namespace System;
using namespace EditorNative;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Camera::Camera( EditorScene ^ _scene )
: scene( _scene ),
	prevPosition ( new NScene::SCameraPosition() )
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Camera::~Camera()
{
	this->!Camera();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Camera::!Camera()
{
	delete prevPosition;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Camera::Yaw::get()
{
	return Position.fYaw;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Yaw::set( float value )
{
	NScene::SCameraPosition pos = Position;
	pos.fYaw = value;
	Position = pos;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Camera::Pitch::get()
{
	return Position.fPitch;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Pitch::set( float value )
{
	NScene::SCameraPosition pos = Position;
	pos.fPitch = value;
	Position = pos;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Camera::Roll::get()
{
	return Position.fRoll;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Roll::set( float value )
{
	NScene::SCameraPosition pos = Position;
	pos.fRoll = value;
	Position = pos;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Camera::Rod::get()
{
	return Position.fRod;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Rod::set( float value )
{
	NScene::SCameraPosition pos = Position;
	pos.fRod = value;
	Position = pos;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Camera::Fov::get()
{
	return scene->NativeScene->GetCamera()->GetFOV();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Fov::set( float value )
{
	scene->NativeScene->GetCamera()->SetFOV( value );
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3^ Camera::Anchor::get()
{
	return gcnew Vector3( Position.vAnchor );
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Anchor::set( Vector3^ value )
{
	NScene::SCameraPosition pos = Position;
	pos.vAnchor = value->Native;
	Position = pos;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NScene::SCameraPosition Camera::Position::get()
{
	NScene::SCameraPosition result;
	scene->NativeScene->GetCamera()->GetPosition( &result );
	return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Position::set( NScene::SCameraPosition value )
{
	if ( prevPosition != nullptr )
	{
		*prevPosition = value;
	}
	scene->NativeScene->GetCamera()->SetPosition( value );
	FireChangedEvent();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3^ Camera::Location::get()
{
	return gcnew Vector3( Position.GetCameraPos() );
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CinemaMode Camera::ScreenMode::get()
{
	return scene->ScreenMode;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::ScreenMode::set( CinemaMode value )
{
	scene->ScreenMode = value;
	FireChangedEvent();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::FireChangedEvent()
{
  NScene::SCameraPosition campos = Position;
  EditorSound::SetListener( campos.GetCameraPos(), campos.GetCameraDir(), campos.GetCameraUp(), campos.vAnchor );
	ParametersChanged( this, EventArgs::Empty );
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Update()
{
	if ( prevPosition != nullptr && *prevPosition != Position )
	{
		FireChangedEvent();
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3^ Camera::Direction::get()
{
	return gcnew Vector3( Position.GetCameraDir() );
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3^ Camera::UpDirection::get()
{
	return gcnew Vector3( Position.GetCameraUp() );
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3^ Camera::RightDirection::get()
{
	return gcnew Vector3( (-Position.GetCameraUp()) ^ Position.GetCameraDir() );
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Camera::Near::get()
{
	return scene->NativeScene->GetCamera()->GetNear();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Near::set(float value)
{
	NScene::ICamera& c = Native;
	return c.SetPlanes(value, c.GetFar());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float Camera::Far::get()
{
	return scene->NativeScene->GetCamera()->GetFar();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::Far::set(float value)
{
	NScene::ICamera& c = Native;
	return c.SetPlanes(c.GetNear(), value);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NScene::ICamera& Camera::Native::get()
{
	return *scene->NativeScene->GetCamera();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
