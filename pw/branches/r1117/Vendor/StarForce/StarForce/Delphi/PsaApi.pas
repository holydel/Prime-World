//
//  PsaApi.pas
//
//  File description:
//      Unit for public API functions. These functions can be used in
//      initialization module as well as in protected application.
//
//  Notes:
//      Detailed info about functions and parameters see in PsaApi.h.
//

unit PsaApi;

interface

uses
  Windows;

// Additional constants and definitions
{$I PsConfig}
{$I PsConstants}

// Additional types
type
  Size_T = Cardinal;
  PSize_T = ^Size_T;
{$IFDEF VER130}
  PBoolean = ^Boolean; // Not present in Delphi 5
{$ENDIF}
  PPointer = ^Pointer;

// License version (see PSA_GetLicenseInformation)
const
  LIC_VERSION_UNDEFINED                             = $FFFFFFFF;
  LIC_VERSION_0_DISC                                = 0;
  LIC_VERSION_1_HARDWARE                            = 1;

// License types (see PSA_GetLicenseInformation)
const
  LIC_TYPE_UNDEFINED                                = $FFFFFFFF;
  LIC_TYPE_HARDWARE                                 = 0;
  LIC_TYPE_HARDWARE_TRIAL                           = 1;
  LIC_TYPE_HARDWARE_RESCUE                          = 2;
  LIC_TYPE_HARDWARE_AUTOGENERATED_AFTER_DISC_CHECK  = 3;
  LIC_TYPE_DISC                                     = 4;

// Codes of independent features that can be granted by license
const
  PSA_GrantedFeature0                               = $00000001;
  PSA_GrantedFeature1                               = $00000002;
  PSA_GrantedFeature2                               = $00000004;
  PSA_GrantedFeature3                               = $00000008;
  PSA_GrantedFeature4                               = $00000010;
  PSA_GrantedFeature5                               = $00000020;
  PSA_GrantedFeature6                               = $00000040;
  PSA_GrantedFeature7                               = $00000080;
  PSA_GrantedFeature8                               = $00000100;
  PSA_GrantedFeature9                               = $00000200;
  PSA_GrantedFeature10                              = $00000400;
  PSA_GrantedFeature11                              = $00000800;
  PSA_GrantedFeature12                              = $00001000;
  PSA_GrantedFeature13                              = $00002000;
  PSA_GrantedFeature14                              = $00004000;
  PSA_GrantedFeature15                              = $00008000;
  PSA_GrantedFeature16                              = $00010000;
  PSA_GrantedFeature17                              = $00020000;
  PSA_GrantedFeature18                              = $00040000;
  PSA_GrantedFeature19                              = $00080000;
  PSA_GrantedFeature20                              = $00100000;
  PSA_GrantedFeature21                              = $00200000;
  PSA_GrantedFeature22                              = $00400000;
  PSA_GrantedFeature23                              = $00800000;
  PSA_GrantedFeature24                              = $01000000;
  PSA_GrantedFeature25                              = $02000000;
  PSA_GrantedFeature26                              = $04000000;
  PSA_GrantedFeature27                              = $08000000;
  PSA_GrantedFeature28                              = $10000000;
  PSA_GrantedFeature29                              = $20000000;
  PSA_GrantedFeature30                              = $40000000;
  PSA_GrantedFeature31                              = $80000000;

//
//	Common functions
//

// Dummy reference
procedure PSA_DummyFunction; stdcall; external 'sakijapi.dll';

// Uninitialize protection core resources
function PSA_Uninitialize: Longword; stdcall; external 'sakijapi.dll';

{$IFDEF PS_USE_LICENSES}

// Get registry path and base handle for license
function PSA_GetLicenseStoragePath(
  pathBuffer: PWideChar;
  pathBufferSizeInWideChars: PSize_T;
  registryBaseHandle: PHandle
  ): Longword; stdcall; external 'sakijapi.dll';

// Get special information for current license
function PSA_GetLicenseInformation(
  licenseVersion: PLongword;
  licenseType: PLongword;
  nonCommercial: PBoolean
  ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}

//
//	Limitation of functionality
//

{$IFDEF PS_USE_LICENSES}

// Get a set of features granted by license (bit mask)
function PSA_GetFeaturesGrantedByLicense(
  features: PLongword
  ): Longword; stdcall; external 'sakijapi.dll';

// Disable features granted by license
function PSA_DisableFeaturesGrantedByLicense(
  features: Longword
  ): Longword; stdcall; external 'sakijapi.dll';

// Check rights granted by license and display error dialog on error
function PSA_CheckFeaturesGrantedByLicense(
  features: Longword
  ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}

//
//	Program start mode
//

{$IFDEF PS_USE_LICENSES}

// Check if the application is running in demo mode
function PSA_IsDemoMode(
  isDemoMode: PBoolean
  ): Longword; stdcall; external 'sakijapi.dll';

// Check if the application is running in trial mode
function PSA_IsTrialMode(
  isTrialMode: PBoolean
  ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}

//
//	Expiration parameters of license
//

{$IFDEF PS_USE_LICENSES}

// Check if the license is already expired
function PSA_IsLicenseExpired(
  isLicenseExpired: PBoolean;
  updateRunTimeData: Boolean
  ): Longword; stdcall; external 'sakijapi.dll';

// Get the time before expiration of license
function PSA_GetTimeToLicenseExpiration(
  timeToLicenseExpiration: PInt64
  ): Longword; stdcall; external 'sakijapi.dll';

// Get date and time of license expiration
function PSA_GetLicenseExpirationDateTime(
  licenseExpirationDateTime: PInt64
  ): Longword; stdcall; external 'sakijapi.dll';

// Get life time limit of license
function PSA_GetLicenseLifeTimeLimit(
  licenseLifeTimeLimit: PInt64
  ): Longword; stdcall; external 'sakijapi.dll';

// Get remaining number of runs
function PSA_GetRemainingNumberOfRuns(
  remainingNumberOfRuns: PLongword
  ): Longword; stdcall; external 'sakijapi.dll';

// Get limit for number of runs
function PSA_GetLicenseNumberOfRunsLimit(
  licenseNumberOfRunsLimit: PLongword
  ): Longword; stdcall; external 'sakijapi.dll';

// Get remaining execution time of application, calculated in the current moment
function PSA_GetRemainingExecutionTime(
  remainingExecutionTime: PInt64
  ): Longword; stdcall; external 'sakijapi.dll';

// Get remaining execution time of application, calculated in the moment of start of application
function PSA_GetRemainingExecutionTimeAtStart(
  remainingExecutinTimeAtStart: PInt64
  ): Longword; stdcall; external 'sakijapi.dll';

// Get limit for execution time of application
function PSA_GetLicenseExecutionTimeLimit(
  licenseExecutionTimeLimit: PInt64
  ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}

//
//	User-defined data in the key
//

{$IFDEF PS_USE_LICENSES}

// Get user-defined field
function PSA_GetUserDefinedField16Bits(
  userDefinedField: PWord
  ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}

//
//	Specific functions for disc binding
//

{$IFDEF BINDING_DISC}

// Get the label of disc with that the check was passed
function PSA_GetDiscLabel(
  labelBuffer: PWideChar;
  labelBufferSizeInWideChars: PSize_T
  ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}

// 
// Specific functions for external binding
// 

{$IFDEF PS_EXTERNAL_BASED_INITIALIZATION_SUPPORTED}

//	Description:
//		Set external binding check success flag. By default flag 
//		is cleared.
//
//	Output:
//		Return value				- PSC_STATUS_SUCCESS
//
function PSA_ExternalBindingCheckSetSucceedFlag:
  Longword; stdcall; external 'sakijapi.dll';

//	Description:
//		Clear external binding check success flag. By default flag 
//		is cleared.
//
//	Output:
//		Return value				- PSC_STATUS_SUCCESS
//
function  PSA_ExternalBindingCheckSetFailedFlag:
  Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}

{$IFDEF PS_SUPPORT_FILE_CHECK_API}

//  Description:
//		Returns result of loaded files checks
//  Input:
//		fileCheckResultOk 			- pointer to output variable
//									  set to true if all of protected files loaded not corrupted
//									  set to false if hash check for one or more files failed 
//	Output:
//		Return value				- PSC_STATUS_SUCCESS
function PSA_GetCheckFilesHashResult(
  fileCheckResultOk: PBoolean
  ): Longword; stdcall; external 'sakijapi.dll';
{$ENDIF}

{$IFDEF PS_SUPPORT_MEMORY_CHECK_API}
//  Description:
//		Checks read only memory regions of protected modules, calculates hash and 
//		compare to hash value stored in protection library
//  Input:
//		memCheckResultOk 			- pointer to output variable
//									  set to true if read only memory for all protected modules remains unchanged
//									  set to false if memory changed for one or more modules
//	Output:
//		Return value				- PSC_STATUS_SUCCESS
function PSA_CheckProtectedModulesReadOnlyMem(
  memCheckResultOk: PBoolean
  ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}

{$IFDEF OPTION_ENABLE_EMBEDDED_USER_DATA}
//  Description:
//		Get embedded data block from protection library
//	Input:
//		blockId 		- id of block to get, id must be in range 0..63
//		data			- pointer to data buffer, if need to get block size pass NULL
//		dataSize		- on input contains size of data buffer, on output size of data written to buffer
//	Output:	
//		return value 	- PSC_STATUS_SUCCESS if id is in allowed range and data is copied to output buffer
//						- PSC_STATUS_UNSUCCESSFUL if id is not in allowed range
//						- PSC_STATUS_BUFFER_TOO_SMALL if buffer is null or buffer is not big enough, dataSize contains size of buffer needed

function PSA_GetEmbeddedDataBlock( 
	blockId: Longword;
	data: PChar;
	dataSize: Size_T
	): Longword; stdcall; external 'sakijapi.dll';	
{$ENDIF}

{$IFDEF OPTION_ENABLE_CLIENT_VALIDATION}
//  Description:
//		Calculate hash on input buffer
//	Input:
//		input 	- buffer with input data, must have size PSA_VALIDATE_CLIENT_BUFFER_SIZE 
//		output	- pointer to buffer for hash calculated on input buffer, must be at least PSA_VALIDATE_CLIENT_BUFFER_SIZE
//
//	Output:	
//		return value 	- PSC_STATUS_SUCCESS or error code

function PSA_ValidateClient(
	input: PChar;
	output: PChar ): Longword; stdcall; external 'sakijapi.dll';
	
{$ENDIF}

{$IFDEF PS_SUPPORT_HARDWARE_CODE_API}

//  Description:
//      Generate the hardware code
///
//  Note:
//      The hardware code is a binary data, not a string. Data size may vary between versions.
//
//  Input:
//      hardwareCodeBuffer          - buffer for storing the hardware code (may be null)
//      hardwareCodeBufferSize      - pointer to variable that holds size of code buffer
//
//  Output:
//      Return value                - PSC_STATUS_SUCCESS or error code ( PSC_STATUS_BUFFER_TOO_SMALL if buffer is too small )
//      *hardwareCodeBuffer         - hardware code
//      *hardwareCodeBufferSize     - used buffer size ( or required buffer size if return value is PSC_STATUS_BUFFER_TOO_SMALL)

function PSA_GetHardwareCode(
  hardwareCodeBuffer: PChar;
  hardwareCodeBufferSize: PSize_T ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}

{$IFDEF OPTION_ENABLE_SYSTEM_LIBS_LOCATION_CHECK}
//  Description:
//		Function performs check that system libraries(list of libraries is defined in protection options) are located in windows system catalog.
//		To get only fact of system libraries replacement call PSA_CheckSystemLibsLocation( null, null, &systemLibLocationChanged )
//		To get full paths to replaced system libraries pass also valid outBuffer and outBufferSizePtr parameters
//	Input:
//		outBuffer 		- 	Buffer that will contain check result - list of wide characters strings separated by zero wide character, each
//							string is full path to library( including module name ) that is not located in system catalog.
//							If outBuffer is null and outBufferSizePtr is not null, value of outBufferSizePtr will contain necessary buffer size on exit.
//		outBufferSizePtr	-	Pointer to size of outBuffer array, on input contains size of array, 
//								after function returns contains used buffer size.
//								If outBufferSizePtr is null, result is returned in systemLibLocationChangedPtr only.
//		systemLibLocationChangedPtr	-  	Pointer to boolean var that will contain check result. Must not be null.
//										Contains true on exit if one of system libraries( defined in protection options )
//										is located outside of windows system directory. Contains false otherwise. 
//	Output:	
//		return value 	- PSC_STATUS_SUCCESS if function performed successfully
//						- PSC_STATUS_BUFFER_TOO_SMALL if outBuffer is not big enough to save result, outBufferSizePtr will contain size of buffer needed, 
//							value of systemLibLocationChangedPtr is valid in this case
//						- PSC_STATUS_INVALID_PARAMETER if systemLibLocationChangedPtr is null
//						- PSC_STATUS_ERROR_IN_WINAPI if one of modules is not loaded or winapi error
//						- other errors
function PSA_CheckSystemLibsLocation(
	outBuffer: PChar;
	outBufferSizePtr: PSize_T;
	systemLibLocationChangedPtr: PBoolean ): Longword; stdcall; external 'sakijapi.dll';
{$ENDIF}

{$IFDEF OPTION_ENABLE_SYSTEM_LIBS_CODE_SECTION_CHECK}

//  Description:
//		Function performs check that read only sections are not modified in system DLLs( list of DLLs is defined in protection options )
//		To get only fact of sections modifications call PSA_CheckSystemLibsReadOnlySections( null, null, &readOnlySectionsModified )
//		To get full information on modified regions pass also valid outBuffer and outBufferSizePtr parameters
//	Input:
//		outBuffer 		- 	Buffer that will contain check result - serialized array of LibraryWithModifiedReadOnlySectionInfo structures 
// 							Format of output buffer is:
//
// 								UInt16 	NumLibsWithSectionsModified - 16 bit value, number of libraries with modified read only sections found
//								followed by sequence of LibraryWithModifiedReadOnlySectionInfo structures, 
//								number of elements in sequence is NumLibsWithSectionsModified
//								where each structure LibraryWithModifiedReadOnlySectionInfo is serialized as followed
//
// 									struct
// 									{
// 										WideChar* 	LibraryName; 			// Zero terminated wide char string, library name
//										UInt32		FileVersionLow;		// 32 bit value, file version low part
//										UInt32		FileVersionHigh;		// 32 bit value, file version high part
// 										UInt32		NumRegions;			// 32 bit value, size of modifications array that follows
//										ModifiedRegion[ NumRegions ];	// array of ModifiedRegion structures, size of array is NumRegions
// 									}
// 									LibraryWithModifiedReadOnlySectionInfo;
//
//									where each structure ModifiedRegion is serialized as followed
//
// 									struct
// 									{
//										UInt32 	RegionRva;  	// 32 bit value - rva of modified region
//										UInt32 	RegionSize;	// 32 bit value - size of modified region
// 									}
// 									ModifiedRegion;
//
//							If outBuffer is null and outBufferSizePtr is not null, outBufferSizePtr will contain necessary buffer size on exit.
//		outBufferSizePtr 	-	Pointer to size of outBuffer array, on input contains size of array, after function returns contains used buffer size.
//							If outBufferSizePtr is null, result is returned in readOnlySectionsModifiedPtr only.
//		readOnlySectionsModifiedPtr	-  	Pointer to boolean var that will contain check result. Must not be null.
//										Contains true on exit if read only sections are modified in one of system libraries(defined in protection options).
//										Contains false otherwise. 
//	Output:	
//		return value 	- PSC_STATUS_SUCCESS if function performed successfully
//						- PSC_STATUS_BUFFER_TOO_SMALL if outBuffer is not big enough to save result, outBufferSizePtr will contain size of buffer needed
//						- PSC_STATUS_INVALID_PARAMETER if readOnlySectionsModifiedPtr is null
//						- other errors 	
function PSA_CheckSystemLibsReadOnlySections(
	outBuffer: PChar;
	outBufferSizePtr: PSize_T;
	readOnlySectionsModifiedPtr: PBoolean ): Longword; stdcall; external 'sakijapi.dll';
	
{$ENDIF}


{$IFDEF OPTION_ENABLE_SYSTEM_LIBS_IAT_CHECK}

//  Description:
//		Function performs check that IAT in system libraries(list of libraries is defined in protection options) are not modified
//		To get only fact of IAT modification call PSA_CheckSystemLibsIat( null, null, &systemLibIatModifiedPtr )
//		To get full info on IAT modifications found, pass also valid outBuffer and outBufferSize parameters
//	Input:
//		outBuffer 		- 	Buffer that will contain check result - format of result info is following:
//							
//								UInt16 NumberOfDllsWithIatModified -  16 bit value, number of ModifiedDllInfo structures 
//								followed by sequence of ModifiedDllInfo structure, number of elements in sequence is NumberOfDllsWithIatModified
//								
//								where each ModifiedDllInfo structure is serialized as followed:
//									struct
//									{
//										WideChar* NameOfDllWithModifiedIAT;					// Zero terminated wide string, name of library with IAT modified
//										UInt16 NumberOfIatModifications; 					// 16 bit value, number of modifications found in IAT
//										ModifiedIatEntryInfo[ NumberOfIatModifications ]	// array of of ModifiedIatEntryInfo structures, size is NumberOfIatModifications
//									}ModifiedDllInfo;
//
//									where ModifiedIatEntryInfo structure has following format:
//									struct
//									{
//										WideChar*	ImportedDllName;				// Zero terminated wide string, name of library, function imported from it was patched
//										UInt8		FuncIsImportedByOrdinal;		// 1 byte value, if not 0 indicates that modified entry in IAT was imported by ordinal 
//										// if FuncIsImportedByOrdinal == 0 next follows 
//										WideChar*	ImportedFunctionName;			// Zero terminated wide string, name of function that was pathced in IAT
//										// if FuncIsImportedByOrdinal != 0 next follows
//										UInt16		Ordinal						// 16 bit value, ordinal of function 
//									}ModifiedIatEntryInfo;
//									
//							If outBuffer is null and outBufferSize is not null, outBufferSize will contain necessary buffer size on exit.
//		outBufferSizePtr	-	Pointer to size of outBuffer array, on input contains size of array, after function returns contains used buffer size.
//								If outBufferSize is null, result is returned in systemLibIatModifiedPtr only.
//		systemLibIatModifiedPtr	-  	Pointer to boolean var that will contain check result. Must not be null.
//										Contains true on exit if IAT is modifies in one of system libraries(defined in protection options).
//										Contains false otherwise. 
//	Output:	
//		return value 	- PSC_STATUS_SUCCESS if function performed successfully
//						- PSC_STATUS_BUFFER_TOO_SMALL if outBuffer is not big enough to save result, outBufferSize will contain size of buffer needed, 
//							value of systemLibIatModified is valid in this case
//						- PSC_STATUS_INVALID_PARAMETER if systemLibIatModifiedPtr is null
//						- PSC_STATUS_ERROR_IN_WINAPI if one of modules is not loaded or winapi error
//						- other errors 
function PSA_CheckSystemLibsIat(
	outBuffer: PChar;
	outBufferSizePtr: PSize_T;
	systemLibIatModifiedPtr: PBoolean ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}




{$IFDEF PS_MQL4_PROTECT}

//  Description:
//		Function performs activation to mql4 script protection
//	Input:
//		accountNumber - account number provided by script
//	Output:	
//		return value - PSC_STATUS_SUCCESS - if activation successfully finished
//		- error codes
function PSA_InfoProperty( accountNumber: Longword ): Longword; stdcall; external 'sakijapi.dll';

{$ENDIF}




{$IFDEF PS_USE_LICENSES}
//
//	Description:
//		Get license creation time
//
//	Input:
//		licenseCreationDateTime		- buffer for time
//
//	Output:
//		Return value				- PSC_STATUS_SUCCESS or error code
//		*licenseCreationDateTime	- license creation time in 100 ns intervals
//
//	Registry keys for debugging:
//		"LicenseCreationTime" of type REG_BINARY and length 8 bytes
//		(initialized at loading of sakijapi.dll).
//
function PSA_GetLicenseCreationDateTime(
	licenseCreationDateTime: PInt64 ): Longword; stdcall; external 'sakijapi.dll';
	
	
//
//	Description:
//		Get license start time
//
//	Input:
//		licenseStartDateTime		- buffer for time
//
//	Output:
//		Return value				- PSC_STATUS_SUCCESS or error code
//		*licenseStartDateTime		- license creation time in 100 ns intervals
//
//	Registry keys for debugging:
//		"LicenseStartDateTime" of type REG_BINARY and length 8 bytes
//		(initialized at loading of sakijapi.dll).
//
function PSA_GetLicenseStartDateTime(
	licenseStartDateTime: PInt64 ): Longword; stdcall; external 'sakijapi.dll';


//
//	Description:
//		Get number of connections.
//
//	Input:
//		numberOfConnections		- buffer for user-defined field
//
//	Output:
//		Return value			- PSC_STATUS_SUCCESS or error code
//		*numberOfConnections	- number of connections (~0ui32 if undefined)
//
//	Registry keys for debugging:
//		"NumberOfConnections" of type REG_DWORD (initialized at loading
//		of sakijapi.dll).
//
function PSA_GetNumberOfConnections(
	numberOfConnections: PLongword ): Longword; stdcall; external 'sakijapi.dll';

	
//
//	Description:
//		Check if license is activated as trial.
//
//	Input:
//		licenseIsActivatedAsTrial		- buffer for license trial mode flag
//
//	Output:
//		Return value	- PSC_STATUS_SUCCESS or error code
//		*licenseIsActivatedAsTrial	- license trial mode flag
//
//	Registry keys for debugging:
//		"LicenseIsActivatedAsTrial" of type REG_DWORD (initialized at loading of sakijapi.dll).
//
function PSA_IsLicenseActivatedAsTrial(
	licenseIsActivatedAsTrial: PBoolean ): Longword; stdcall; external 'sakijapi.dll';

	
{$ENDIF}

{$IFDEF PS_SUPPORT_CRYPTED_TRAFFIC}
// Public RSA key
type 
	PSA_CryptedTrafficKey = Record
		KeySizeInBytes: Size_T;			// Size of RSA key (current implementation
										// support only 128-byte or 256-byte keys)
		Modulus: PByte;					// Little-endian, size is KeySizeInBytes
		Exponent: PByte;				// Public exponent, little-endian, size is
										// KeySizeInBytes
	end;
	PPSA_CryptedTrafficKey = ^PSA_CryptedTrafficKey;


//
//  Description:
//		Open strem of crypted traffic. The function creates initialization
//		block that should be sent to the remote server as first portion of
//		data.
//
//  Input:
//		randomSeed				    - random seed for session key (can be NULL)
//      randomSeedSize				- size of random seed in bytes (can be 0)
//		key							- public key structure to encrypt
//			initialization block
//		initializationBlock			- initialization block buffer
//		initializationBlockSize		- size of buffer
//
//	Output:
//		Return value				- PSC_STATUS_SUCCESS or error code
//			(PSC_STATUS_BUFFER_TOO_SMALL if buffer is too small)
//  	streamHandle				- stream handle
//		initializationBlockSize		- really used (or required, if return value
//			is PSC_STATUS_BUFFER_TOO_SMALL) size of initialization block
//
//	Note:
//		1.	If randomSeed is NULL or randomSeedSize is 0, the session key will be
//			initialized from timer.
//		2. 	Multiple streams are not supported in current implementation.
//
function PSA_CryptedTrafficOpen(
	const randomSeed: PChar;
	randomSeedSize: Size_T;
	key: PPSA_CryptedTrafficKey;
	streamHandle: PPointer;
	initializationBlock: PChar;
	initializationBlockSize: PSize_T ): Longword; stdcall; external 'sakijapi.dll';


//
//  Description:
//		Encrypt byte sequence.
//
//  Input:
//		streamHandle				- stream handle to encrypt bytes for
//      inputData					- input data buffer
//		outputData					- output data buffer
//		dataSize					- length of data in bytes
//
//	Output:
//		Return value				- PSC_STATUS_SUCCESS or error code
//
function PSA_CryptedTrafficEncrypt(
	streamHandle: Pointer;
	inputData: PChar;
	outputData: PChar;
	dataSize: Size_T): Longword; stdcall; external 'sakijapi.dll';

	
//
//  Description:
//		Decrypt byte sequence.
//
//  Input:
//		streamHandle				- stream handle to decrypt bytes for
//      inputData					- input data buffer
//		outputData					- output data buffer
//		dataSize					- length of data in bytes
//
//	Output:
//		Return value				- PSC_STATUS_SUCCESS or error code
//
function PSA_CryptedTrafficDecrypt(
	streamHandle: Pointer;
	inputData: PChar;
	outputData: PChar;
	dataSize: Size_T): Longword; stdcall; external 'sakijapi.dll';


//
//	Description:
//		Close stream of crypted traffic.
//
//  Input:
//		streamHandle				- stream handle to close
//
//	Output:
//		Return value				- PSC_STATUS_SUCCESS or error code
//
function PSA_CryptedTrafficClose(
	streamHandle: Pointer ): Longword; stdcall; external 'sakijapi.dll';

	
{$ENDIF}


{$IFDEF DETECT_SUSPICIOUS_VECTORED_EXCEPTIONS}
function PSA_CheckSuspiciousVectoredExceptions( 
	suspiciousVehResultOk: PBoolean ): Longword; stdcall; external 'sakijapi.dll';	
{$ENDIF}

{$IFDEF PS_SUPPORT_CERTIFICATE_CHECK}

//
//	Description:
//		Generate certificate request packet for sending to the server.
//
//	Input:
//		requestBuffer	 	- buffer for request
//		requestBufferSize	- size of buffer in bytes
//
//	Output:
//		Return value				- PSC_STATUS_SUCCESS or error code
//			(PSC_STATUS_BUFFER_TOO_SMALL if buffer is too small)
//		*requestBuffer				- certificate request
//		*requestBufferSize			- used buffer size in bytes (or required
//			buffer size if return value is PSC_STATUS_BUFFER_TOO_SMALL)
//
function PSA_CertificateGenerateRequest(
	requestBuffer: PChar;
	requestBufferSize: PSize_T ): Longword; stdcall; external 'sakijapi.dll';
	
//
//	Description:
//		Validate certificate sent from server.
//
//	Input:
//		responseBuffer				- buffer with response from the server
//		responseBufferSize			- size of buffer in bytes
//
//	Output:
//		returnValue					- PSC_STATUS_SUCCESS of error code
//			(PSC_STATUS_INVALID_PARAMETER if arguments are invalid,
//			PSC_STATUS_CERTIFICATE_INVALID_FORMAT
//				if format of certificate is invalid or validation is failed,
//			PSC_STATUS_CERTIFICATE_NOT_VALID_YET
//				if validity period of certificate is in future,
//			PSC_STATUS_CERTIFICATE_NOT_VALID_ALREADY
//				if validity period of certificate is in past )
//
function PSA_CertificateValidate(
	responseBuffer: PChar;
	responseBufferSize: Size_T ): Longword; stdcall; external 'sakijapi.dll';
	
{$ENDIF}


{$IFDEF PS_FILE_SYSTEM_USER_MODE}

// File position moving origin definition.
Const
  PSA_FS_SEEK_SET	= 0;
  PSA_FS_SEEK_CUR	= 1;
  PSA_FS_SEEK_END	= 2;

//	Description:
//		Open a protected file.
//
//	Input:
//		fileName		- File name to open.
//		fileHandle		- Pointer to output handle of opened file if call succeed.
//
//	Output:
//		Return value	- PSC_STATUS_SUCCESS if success, other PSC_STATUS code if failure.
//
function PSA_FsOpenFile(
	const fileName: PWChar;
	fileHandle: PPointer ): Longword; stdcall; external 'sakijapi.dll';

//	Description:
//		Close a protected file.
//
//	Input:
//		fileHandle		- File handle.
//
//	Output:
//		Return value	- PSC_STATUS_SUCCESS if success, other PSC_STATUS code if failure.
//
function PSA_FsCloseFile(
	fileHandle: Pointer ): Longword; stdcall; external 'sakijapi.dll';

//	Description:
//		Retrieve size of the specified protected file.
//
//	Input:
//		fileHandle		- File handle.
//		fileSize		- Pointer to a variable that receives the file size.
//
//	Output:
//		Return value	- PSC_STATUS_SUCCESS if success, other PSC_STATUS code if failure.
//
function PSA_FsGetFileSize(
	fileHandle: Pointer;
	fileSize: PInt64 ): Longword; stdcall; external 'sakijapi.dll';

//	Description:
//		Move file position of the specified protected file.
//
//	Input:
//		fileHandle		- File handle.
//		origin			- Starting point for the file position move.
//						  This parameter can be one of the following values:
//						    PSA_FS_SEEK_SET - Starting point is the beginning of the file.
//						    PSA_FS_SEEK_CUR - Starting point is the current value of the file position.
//						    PSA_FS_SEEK_END - Starting point is the current end-of-file position.
//		offset			- Distance to move file position from the origin.
//		newOffset		- Pointer to a variable that receives new file position. Can be null.
//
//	Output:
//		Return value	- PSC_STATUS_SUCCESS if success, other PSC_STATUS code if failure.
//
function PSA_FsSetFilePosition(
	fileHandle: Pointer;
	origin: Integer;
	offset: Int64;
	newOffset: PInt64 ): Longword; stdcall; external 'sakijapi.dll';

//	Description:
//		Read data from the specified protected file starting at the current file position.
//
//	Input:
//		fileHandle			- File handle.
//		buffer				- Pointer to a buffer that receives the data.
//		numberOfBytesToRead	- Maximum number of bytes to be read.
//		numberOfBytesRead	- Pointer to a variable that receives number of bytes actually read. Can be null.
//
//	Output:
//		Return value		- PSC_STATUS_SUCCESS if success, other PSC_STATUS code if failure.
//
function PSA_FsReadFile(
	fileHandle: Pointer;
	buffer: Pointer;
	numberOfBytesToRead: Size_T;
	numberOfBytesRead: PSize_T ): Longword; stdcall; external 'sakijapi.dll';

//	Description:
//		Verify digital signature of the specified real file.
//
//	Input:
//		fileName		- File name.
//		valid			- Pointer to a variable that receives result of verification.
//							Nonzero if digital signature of the file is valid, zero if invalid.
//
//	Output:
//		Return value	- PSC_STATUS_SUCCESS if success, other PSC_STATUS code if failure.
//
function PSA_FsVerifyFileSignature(
	const fileName: PWChar;
	valid: PInteger ): Longword; stdcall; external 'sakijapi.dll';
	
{$ENDIF}


implementation

// No implementation

end.
 
