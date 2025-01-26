#pragma once

namespace Network
{

const string & GetCoordinatorAddress();
const string & GetLoginServerAddress();
int GetFirstServerPortBack();
int GetFirstServerPortFront();

string const & GetFrontendIPAddr();
string const & GetBackendIPAddr();

} //namespace Network
