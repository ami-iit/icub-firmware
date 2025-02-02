/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero
 * email:   valentina.gaggero@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#error dont use it in this build

// - include guard ----------------------------------------------------------------------------------------------------
#ifndef _EOTHEEMASAPPLBODY_HID_H_
#define _EOTHEEMASAPPLBODY_HID_H_

#ifdef __cplusplus
extern "C" {
#endif

// - doxy -------------------------------------------------------------------------------------------------------------
/* @file       EOtheEMSApplBody_hid.h
    @brief     This file provides hidden interface to TheEmsAppBody singleton.
    @author    valentina.gaggero@iit.it
    @date      09/19/2012
**/


// - external dependencies --------------------------------------------------------------------------------------------
#include "EoManagement.h"
#include "EOtimer.h"

#include "EOtheCANdiscovery2.h"


// - declaration of extern public interface ---------------------------------------------------------------------------
#include "EOtheEMSapplBody.h"


// - definition of the hidden struct implementing the object ----------------------------------------------------------


typedef enum
{
    eo_emsApplBody_st__NOTinitted   = 0,
    eo_emsApplBody_st__initted      = 1,
} eOtheEMSapplBody_hid_states_t;


struct EOtheEMSapplBody_hid
{
    eOemsapplbody_cfg_t                 config; 
    eOtheEMSapplBody_hid_states_t       st;
    eOmn_appl_runMode_t                 appRunMode;
    EOappEncReader                      *appEncReader;
    EOemsController                     *emsController;
    eObool_t                            hasdevice[eo_emsapplbody_deviceid_numberof];
    
    eOemsapplbody_MCtype                mctype;
    eObool_t                            isMCall_ready;
    eObool_t                            isASstrain_ready;
    eObool_t                            isASmais_ready;
    
    eObool_t                            BOARDisreadyforcontrolloop;
    //eOcandiscovery_detection_t          failedDetection;
}; 


// - declaration of extern hidden functions ---------------------------------------------------------------------------


#ifdef __cplusplus
}       // closing brace for extern "C"
#endif 
 
#endif  // include-guard

// - end-of-file (leave a blank line after)----------------------------------------------------------------------------



