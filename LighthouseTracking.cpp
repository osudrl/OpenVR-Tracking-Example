#include "LighthouseTracking.h"
#include <stdio.h>



// Destructor
LighthouseTracking::~LighthouseTracking() 
{
	if (vr_pointer != NULL)
	{
		VR_Shutdown();
		vr_pointer = NULL;
	}
}

// Constructor
LighthouseTracking::LighthouseTracking() 
{
	// Definition of EVRInitError local variable
	EVRInitError eError = VRInitError_None;

	/*
	VR_Init (
	  	arg1: Pointer to vr:EVRInitError type (I think its an enum)
	  	arg2: Must be of type vr::EVRApplicationType

	  		The type of VR Applicaion.  This example uses the SteamVR instance that is already running.  
	        Because of this, the init function will fail if SteamVR is not already running. 

	        Other EVRApplicationTypes include:
	        	* VRApplication_Scene - "A 3D application that will be drawing an environment.""
	        	* VRApplication_Overlay - "An application that only interacts with overlays or the dashboard.""
	        	* VRApplication_Utility
	*/

	vr_pointer = VR_Init(&eError, VRApplication_Background);

	// If the init failed because of the error
	if (eError != VRInitError_None)
	{
		vr_pointer = NULL;
		printf("Unable to init VR runtime: %s \n", VR_GetVRInitErrorAsEnglishDescription(eError));
		exit(EXIT_FAILURE);
	}
}

/*
	defined fuction RunProcedure()

	* Loop-listen for events then parses them (e.g. prints the to user)
	* Returns true if success or false if openvr has quit
	* Should be called by main.cpp with the bShouldWaitForEvents flag
*/
bool LighthouseTracking::RunProcedure(bool bShouldWaitForEvents) 
{

	

	// Either A) wait for events, such as hand controller button press, before parsing...
	if (bShouldWaitForEvents) 
	{
		// Define a VREvent
		VREvent_t event;

		/* 
			{ vr::IVRSystem. , vr_pointer-> } PollNextEvent(

				arg1: VREvent_t*  A pointer to a VREvent_t which will be filled
				arg2: Size of the VREvent_t struct in bytes
				
				If there is an event, returns ture and fills the event pointer with the
				next event.  If no event, returns false.

				I think the function "hangs" until there is an event, because tracking
				data has only been printed right after some event happens.
		*/

		while (vr_pointer->PollNextEvent(&event, sizeof(event)))
		{
			/*
				ProcessVREvent is a function defined in this module.  It returns false if
				the function determines the type of error to be fatal or signal some kind of quit.
			*/
			if (!ProcessVREvent(event)) 
			{
				// If ProcessVREvent determined that OpenVR quit, print quit message
				printf("\n(OpenVR) service quit");
				return false;
			}
			ParseTrackingFrame();
		}
	}
	// ... or B) continous parsint of tracking data irrespective of events
	else 	
		ParseTrackingFrame();
	
	return true;
}

/*
	defined fuction ProcessVREvent()

	* Takes a single vr:VREvent_t and processes the result
	* Returns false if OpenVR has quit, otherwise returns true
*/
bool LighthouseTracking::ProcessVREvent(const VREvent_t & event)
{
	switch (event.eventType)
	{
		case VREvent_TrackedDeviceActivated:
			 printf("\n(OpenVR) Device : %d attached", event.trackedDeviceIndex);
		break;

		case VREvent_TrackedDeviceDeactivated:
			printf("\n(OpenVR) Device : %d detached", event.trackedDeviceIndex);
		break;

		case VREvent_TrackedDeviceUpdated:
			printf("\n(OpenVR) Device : %d updated", event.trackedDeviceIndex);
		break;

		case VREvent_DashboardActivated:
			printf("\n(OpenVR) Dashboard activated");
		break;

		case VREvent_DashboardDeactivated:
			printf("\n(OpenVR) Dashboard deactivated");
		break;

		case VREvent_ChaperoneDataHasChanged:
			printf("\n(OpenVR) Chaperone data has changed");
		break;

		case VREvent_ChaperoneSettingsHaveChanged:
			printf("\n(OpenVR) Chaperone settings have changed");
		break;

		case VREvent_ChaperoneUniverseHasChanged:
			printf("\n(OpenVR) Chaperone universe has changed");
		break;

		case VREvent_ApplicationTransitionStarted:
			printf("\n(OpenVR) Application Transition: Transition has started");
		break;

		case VREvent_ApplicationTransitionNewAppStarted:
			printf("\n(OpenVR) Application transition: New app has started");
		break;

		case VREvent_Quit:
		{
			printf("\n(OpenVR) Received SteamVR Quit (%d%s", VREvent_Quit, ")");
			return false;
		}
		break;

		case VREvent_ProcessQuit:
		{
			printf("\n(OpenVR) SteamVR Quit Process (%d%s", VREvent_ProcessQuit, ")");
			return false;
		}
		break;

		case VREvent_QuitAborted_UserPrompt:
		{
			printf("\n(OpenVR) SteamVR Quit Aborted UserPrompt (%d%s", VREvent_QuitAborted_UserPrompt, ")");
			return false;
		}
		break;

		case VREvent_QuitAcknowledged:
		{
			printf("\n(OpenVR) SteamVR Quit Acknowledged (%d%s", VREvent_QuitAcknowledged, ")");
			return false;
		}
		break;

		case VREvent_TrackedDeviceRoleChanged:
			printf("\n(OpenVR) TrackedDeviceRoleChanged: %d", event.trackedDeviceIndex);
		break;

		case VREvent_TrackedDeviceUserInteractionStarted:
			printf("\n(OpenVR) TrackedDeviceUserInteractionStarted: %d", event.trackedDeviceIndex);
		break;
		
		default:
			if (event.eventType >= 200 && event.eventType <= 203)
				printf("\n(OpenVR) ButtonEvent#: %d", event.eventType);
			else
				printf("\n(OpenVR) Event: %d", event.eventType);
	}

	return true;
}

HmdVector3_t LighthouseTracking::GetPosition(HmdMatrix34_t matrix) 
{
	HmdVector3_t vector;

	vector.v[0] = matrix.m[0][3];
	vector.v[1] = matrix.m[1][3];
	vector.v[2] = matrix.m[2][3];

	return vector;
}

void LighthouseTracking::ParseTrackingFrame() 
{
	/*
		This for loop will iterate over all of the tracked devices.
		* deviceId is the locaL variable holding the index.
	*/
	for (unsigned int deviceId; deviceId < k_unMaxTrackedDeviceCount; deviceId++)
	{
	
		// if not connected just skip the rest of the routine
		if (!vr_pointer->IsTrackedDeviceConnected(deviceId))
			continue;

		TrackedDevicePose_t trackedDevicePose;
		VRControllerState_t controllerState;
		HmdVector3_t position;

		if (vr_pointer->IsInputFocusCapturedByAnotherProcess())
			printf( "\nInput Focus by Another Process");

		// Get what type of device it is and work with its data
		ETrackedDeviceClass trackedDeviceClass = vr_pointer->GetTrackedDeviceClass(deviceId);
		if (trackedDeviceClass == ETrackedDeviceClass::TrackedDeviceClass_HMD)
		{
			vr_pointer->GetDeviceToAbsoluteTrackingPose(TrackingUniverseStanding, 0, &trackedDevicePose, 1);
			position = GetPosition(trackedDevicePose.mDeviceToAbsoluteTracking);
			printf("\nCOORDS-- HMD x: %.3f y: %.3f z: %.3f", position.v[0], position.v[1], position.v[2]);
		}
		else if (trackedDeviceClass == ETrackedDeviceClass::TrackedDeviceClass_Controller)
		{
			vr_pointer->GetControllerStateWithPose(TrackingUniverseStanding, deviceId, &controllerState, sizeof(controllerState), &trackedDevicePose);
			position = GetPosition(trackedDevicePose.mDeviceToAbsoluteTracking);	
			ETrackedControllerRole role = vr_pointer->GetControllerRoleForTrackedDeviceIndex(deviceId);

			//printf("The mask for %s@'%d' is %d", "k_EButton_Grip", vr::EVRButtonId::k_EButton_Grip, vr::ButtonMaskFromId( vr::EVRButtonId::k_EButton_Grip ));

			if (role == TrackedControllerRole_Invalid)
				continue;
			else if (role == TrackedControllerRole_LeftHand)
				printf(" LEFT x: %.3f y: %.3f z: %.3f", position.v[0], position.v[1], position.v[2]);	
			else if (role == TrackedControllerRole_RightHand)
				printf(" RIGHT x: %.3f y: %.3f z: %.3f", position.v[0], position.v[1], position.v[2]);
		}
			
	}
}
