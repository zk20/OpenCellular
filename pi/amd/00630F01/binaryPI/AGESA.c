/* $NoKeywords:$ */
/**
 * @file
 *
 * Agesa structures and definitions
 *
 * Contains AMD AGESA core interface
 *
 * @xrefitem bom "File Content Label" "Release Content"
 * @e project:      AGESA
 * @e sub-project:  Include
 * @e \$Revision: 85818 $   @e \$Date: 2013-01-11 17:04:21 -0600 (Fri, 11 Jan 2013) $
 */
/*****************************************************************************
 *
 * Copyright (c) 2008 - 2014, Advanced Micro Devices, Inc.
 *               2013 - 2014, Sage Electronic Engineering, LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Advanced Micro Devices, Inc. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***************************************************************************/
#include <AGESA.h>
#include <AcpiLib.h>
#include <amdlib.h>
#include <cbfs.h>
#include <FchCommonCfg.h>
#include <Fch.h>
#include <FchDef.h>
#include <FieldAccessors.h>

#define CONFIG_CBFS_AGESA_NAME "AGESA"

AGESA_STATUS
HeapAllocateBuffer (
  IN OUT   VOID *AllocateHeapParams,
  IN OUT   VOID *StdHeader
  );

AGESA_STATUS
HeapDeallocateBuffer (
  IN       UINT32 BufferHandle,
  IN       VOID *StdHeader
  );

CONST UINT32 ImageSignature = IMAGE_SIGNATURE;
CONST UINT32 ModuleSignature = MODULE_SIGNATURE;
CONST CHAR8 ModuleIdentifier[] = AGESA_ID;

/************************************************************************
 *
 *  AGESA Basic Level interface structure definition and function prototypes
 *
 ***********************************************************************/

static
CONST VOID *
LocateModule (
  IN       CONST CHAR8 name[8]
  )
{
	struct cbfs_media media;
	struct cbfs_file* file;
	const void* agesa;
	size_t agesa_size = 0;
	const AMD_IMAGE_HEADER* image;
	const AMD_MODULE_HEADER* module;

	if (init_default_cbfs_media(&media)) return NULL;
	file = cbfs_get_file(&media, (const char*)CONFIG_CBFS_AGESA_NAME);
	if (!file) return NULL;
	agesa = cbfs_get_file_content(&media, (const char*)CONFIG_CBFS_AGESA_NAME, ntohl(file->type), &agesa_size);
	if (!agesa) return NULL;

	image =  LibAmdLocateImage(agesa, agesa + ntohl(file->len) - 1, 4096, name);
	module = (AMD_MODULE_HEADER*)image->ModuleInfoOffset;

	return module;
}

/**********************************************************************
 * Interface call:  AmdCreateStruct
 **********************************************************************/
AGESA_STATUS
AmdCreateStruct (
  IN OUT   AMD_INTERFACE_PARAMS *InterfaceParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	InterfaceParams->StdHeader.Func = AMD_CREATE_STRUCT;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(InterfaceParams);
}

/**********************************************************************
 * Interface call:  AmdReleaseStruct
 **********************************************************************/
AGESA_STATUS
AmdReleaseStruct (
  IN OUT   AMD_INTERFACE_PARAMS *InterfaceParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	InterfaceParams->StdHeader.Func = AMD_RELEASE_STRUCT;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(InterfaceParams);
}

/**********************************************************************
 * Interface call:  AmdInitReset
 **********************************************************************/
AGESA_STATUS
AmdInitReset (
  IN OUT   AMD_RESET_PARAMS     *ResetParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	ResetParams->StdHeader.Func = AMD_INIT_RESET;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(ResetParams);
}

/**********************************************************************
 * Interface call:  AmdInitEarly
 **********************************************************************/
AGESA_STATUS
AmdInitEarly (
  IN OUT   AMD_EARLY_PARAMS     *EarlyParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	EarlyParams->StdHeader.Func = AMD_INIT_EARLY;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(EarlyParams);
}

/**********************************************************************
 * Interface call:  AmdInitPost
 **********************************************************************/
AGESA_STATUS
AmdInitPost (
  IN OUT   AMD_POST_PARAMS      *PostParams         ///< Amd Cpu init param
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	PostParams->StdHeader.Func = AMD_INIT_POST;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(PostParams);
}

/**********************************************************************
 * Interface call:  AmdInitEnv
 **********************************************************************/
AGESA_STATUS
AmdInitEnv (
  IN OUT   AMD_ENV_PARAMS       *EnvParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	EnvParams->StdHeader.Func = AMD_INIT_ENV;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(EnvParams);
}

/**********************************************************************
 * Interface call:  AmdInitMid
 **********************************************************************/
AGESA_STATUS
AmdInitMid (
  IN OUT   AMD_MID_PARAMS       *MidParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	MidParams->StdHeader.Func = AMD_INIT_MID;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(MidParams);
}

/**********************************************************************
 * Interface call:  AmdInitLate
 **********************************************************************/
AGESA_STATUS
AmdInitLate (
  IN OUT   AMD_LATE_PARAMS      *LateParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	LateParams->StdHeader.Func = AMD_INIT_LATE;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(LateParams);
}

/**********************************************************************
 * Interface call:  AmdInitRecovery
 **********************************************************************/
AGESA_STATUS
AmdInitRecovery (
  IN OUT   AMD_RECOVERY_PARAMS    *RecoveryParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	RecoveryParams->StdHeader.Func = AMD_INIT_RECOVERY;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(RecoveryParams);
}

/**********************************************************************
 * Interface call:  AmdInitResume
 **********************************************************************/
AGESA_STATUS
AmdInitResume (
  IN       AMD_RESUME_PARAMS    *ResumeParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	ResumeParams->StdHeader.Func = AMD_INIT_RESUME;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(ResumeParams);
}

/**********************************************************************
 * Interface call:  AmdS3LateRestore
 **********************************************************************/
AGESA_STATUS
AmdS3LateRestore (
  IN OUT   AMD_S3LATE_PARAMS    *S3LateParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	S3LateParams->StdHeader.Func = AMD_S3LATE_RESTORE;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(S3LateParams);
}

/**********************************************************************
 * Interface call:  AmdS3Save
 **********************************************************************/
AGESA_STATUS
AmdS3Save (
  IN OUT   AMD_S3SAVE_PARAMS    *AmdS3SaveParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	AmdS3SaveParams->StdHeader.Func = AMD_S3_SAVE;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(AmdS3SaveParams);
}

/**********************************************************************
 * Interface call:  AmdLateRunApTask
 **********************************************************************/
AGESA_STATUS
AmdLateRunApTask (
  IN       AP_EXE_PARAMS  *AmdApExeParams
)
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	AmdApExeParams->StdHeader.Func = AMD_LATE_RUN_AP_TASK;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(AmdApExeParams);
}

/**********************************************************************
 * Interface service call:  AmdGetApicId
 **********************************************************************/
AGESA_STATUS
AmdGetApicId (
  IN OUT AMD_APIC_PARAMS *AmdParamApic
)
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	AmdParamApic->StdHeader.Func = AMD_GET_APIC_ID;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(AmdParamApic);
}

/**********************************************************************
 * Interface service call:  AmdGetPciAddress
 **********************************************************************/
AGESA_STATUS
AmdGetPciAddress (
  IN OUT   AMD_GET_PCI_PARAMS *AmdParamGetPci
)
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	AmdParamGetPci->StdHeader.Func = AMD_GET_PCI_ADDRESS;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(AmdParamGetPci);
}

/**********************************************************************
 * Interface service call:  AmdIdentifyCore
 **********************************************************************/
AGESA_STATUS
AmdIdentifyCore (
  IN OUT  AMD_IDENTIFY_PARAMS *AmdParamIdentify
)
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	AmdParamIdentify->StdHeader.Func = AMD_IDENTIFY_CORE;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(AmdParamIdentify);
}

/**********************************************************************
 * Interface service call:  AmdReadEventLog
 **********************************************************************/
AGESA_STATUS
AmdReadEventLog (
  IN       EVENT_PARAMS *Event
)
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	Event->StdHeader.Func = AMD_READ_EVENT_LOG;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(Event);
}

/**********************************************************************
 * Interface service call:  AmdIdentifyDimm
 **********************************************************************/
AGESA_STATUS
AmdIdentifyDimm (
  IN OUT   AMD_IDENTIFY_DIMM *AmdDimmIdentify
)
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	AmdDimmIdentify->StdHeader.Func = AMD_IDENTIFY_DIMMS;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(AmdDimmIdentify);
}

AGESA_STATUS
AmdIdsRunApTaskLate (
  IN       AP_EXE_PARAMS  *AmdApExeParams
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	AmdApExeParams->StdHeader.Func = -1;
	return AGESA_UNSUPPORTED;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(AmdApExeParams);
}

/**********************************************************************
 * Interface service call:  AmdGet2DDataEye
 **********************************************************************/
AGESA_STATUS
AmdGet2DDataEye (
  IN OUT   AMD_GET_DATAEYE *AmdGetDataEye
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	AmdGetDataEye->StdHeader.Func = AMD_GET_2D_DATA_EYE;
	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;
	return Dispatcher(AmdGetDataEye);
}

/**********************************************************************
 * FCH Functions
 **********************************************************************/

VOID FchInitS3LateRestore (IN FCH_DATA_BLOCK *FchDataPtr);
VOID FchInitS3EarlyRestore (IN FCH_DATA_BLOCK *FchDataPtr);

VOID
FchInitS3EarlyRestore (
  IN      FCH_DATA_BLOCK     *FchDataPtr
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	FchDataPtr->StdHeader->Func = FCH_INIT_S3_EARLY_RESTORE;
	if (!module) return;
	Dispatcher = module->ModuleDispatcher;
	Dispatcher(FchDataPtr);
}

VOID
FchInitS3LateRestore (
  IN      FCH_DATA_BLOCK     *FchDataPtr
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	FchDataPtr->StdHeader->Func = FCH_INIT_S3_LATE_RESTORE;
	if (!module) return;
	Dispatcher = module->ModuleDispatcher;
	Dispatcher(FchDataPtr);
}

/**
 * WaitForEcLDN9MailboxCmdAck
 *
 *
 * @param[in] StdHeader
 *
 */
VOID
WaitForEcLDN9MailboxCmdAck (
  IN AMD_CONFIG_PARAMS  *StdHeader
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	StdHeader->Func = 0;
	return;
	if (!module) return;
	Dispatcher = module->ModuleDispatcher;
	Dispatcher(StdHeader);
}

/**
 * ImcSleep - IMC Sleep.
 *
 *
 * @param[in] FchDataPtr Fch configuration structure pointer.
 *
 */
VOID
ImcSleep (
  IN  VOID     *FchDataPtr
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	((FCH_DATA_BLOCK*)FchDataPtr)->StdHeader->Func = 0;
	return;
	if (!module) return;
	Dispatcher = module->ModuleDispatcher;
	Dispatcher(FchDataPtr);
}

/**
 * SoftwareDisableImc - Software disable IMC strap
 *
 *
 * @param[in] FchDataPtr Fch configuration structure pointer.
 *
 */
VOID
SoftwareDisableImc (
  IN  VOID     *FchDataPtr
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	((FCH_DATA_BLOCK*)FchDataPtr)->StdHeader->Func = 0;
	return;
	if (!module) return;
	Dispatcher = module->ModuleDispatcher;
	Dispatcher(FchDataPtr);
}

/**
 * ImcEnableSurebootTimer - IMC Enable Sureboot Timer.
 *
 *
 * @param[in] FchDataPtr Fch configuration structure pointer.
 *
 */
VOID
ImcEnableSurebootTimer (
  IN  VOID     *FchDataPtr
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	((FCH_DATA_BLOCK*)FchDataPtr)->StdHeader->Func = 0;
	return;
	if (!module) return;
	Dispatcher = module->ModuleDispatcher;
	Dispatcher(FchDataPtr);
}

/**
 * ImcDisableSurebootTimer - IMC Disable Sureboot Timer.
 *
 *
 * @param[in] FchDataPtr Fch configuration structure pointer.
 *
 */
VOID
ImcDisableSurebootTimer (
  IN  VOID     *FchDataPtr
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	((FCH_DATA_BLOCK*)FchDataPtr)->StdHeader->Func = 0;
	return;
	if (!module) return;
	Dispatcher = module->ModuleDispatcher;
	Dispatcher(FchDataPtr);
}

/**
 * ImcWakeup - IMC Wakeup.
 *
 *
 * @param[in] FchDataPtr Fch configuration structure pointer.
 *
 */
VOID
ImcWakeup (
  IN  VOID     *FchDataPtr
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	((FCH_DATA_BLOCK*)FchDataPtr)->StdHeader->Func = 0;
	return;
	if (!module) return;
	Dispatcher = module->ModuleDispatcher;
	Dispatcher(FchDataPtr);
}

/**
 * ImcIdle - IMC Idle.
 *
 *
 * @param[in] FchDataPtr Fch configuration structure pointer.
 *
 */
VOID
ImcIdle (
  IN  VOID     *FchDataPtr
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);
	((FCH_DATA_BLOCK*)FchDataPtr)->StdHeader->Func = 0;
	return;
	if (!module) return;
	Dispatcher = module->ModuleDispatcher;
	Dispatcher(FchDataPtr);
}

// TODO This has to be removed
AGESA_STATUS
HeapAllocateBuffer (
  IN OUT   VOID *AllocateHeapParams,
  IN OUT   VOID *StdHeader
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);

	AMD_INTERFACE_PARAMS InterfaceParams = {};

	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;

	InterfaceParams.StdHeader = *(AMD_CONFIG_PARAMS*)StdHeader;
	InterfaceParams.StdHeader.Func = AMD_HEAP_ALLOCATE_BUFFER;

	InterfaceParams.AllocationMethod = PreMemHeap;
	InterfaceParams.NewStructPtr = AllocateHeapParams;

	return Dispatcher(&InterfaceParams);
}

// TODO This has to be removed
AGESA_STATUS
HeapDeallocateBuffer (
  IN       UINT32 BufferHandle,
  IN       VOID *StdHeader
  )
{
	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);

	AMD_INTERFACE_PARAMS InterfaceParams = {};

	if (!module) return AGESA_UNSUPPORTED;
	Dispatcher = module->ModuleDispatcher;

	InterfaceParams.StdHeader = *(AMD_CONFIG_PARAMS*)StdHeader;
	InterfaceParams.StdHeader.Func = AMD_HEAP_DEALLOCATE_BUFFER;

	InterfaceParams.NewStructPtr = &BufferHandle;

	return Dispatcher(&InterfaceParams);
}

/**********************************************************************
 * Interface call:  AmdSetValue
 **********************************************************************/
AGESA_STATUS
AmdSetValue (
  IN       CONST AGESA_FIELD_NAME name,
  IN OUT   VOID* value,
  IN       UINT32 size
  )
{
	AGESA_STATUS status = AGESA_UNSUPPORTED;

	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);

	AMD_ACCESSOR_PARAMS AccessorParams = {};

	if (!module) return status;
	Dispatcher = module->ModuleDispatcher;

	AccessorParams.StdHeader.AltImageBasePtr = 0;
	AccessorParams.StdHeader.CalloutPtr = NULL;
	AccessorParams.StdHeader.Func = AMD_SET_VALUE;
	AccessorParams.StdHeader.ImageBasePtr = 0;

	AccessorParams.AllocationMethod = ByHost;
	AccessorParams.FieldName = name;
	AccessorParams.FieldValue = value;
	AccessorParams.FieldSize = size;

	status = Dispatcher(&AccessorParams);
	return status;
}

/**********************************************************************
 * Interface call:  AmdGetValue
 **********************************************************************/
AGESA_STATUS
AmdGetValue (
  IN       CONST AGESA_FIELD_NAME name,
  IN OUT   VOID** value,
  IN       UINT32 size
  )
{
	AGESA_STATUS status = AGESA_UNSUPPORTED;

	MODULE_ENTRY Dispatcher = NULL;
	const AMD_MODULE_HEADER* module = LocateModule(ModuleIdentifier);

	AMD_ACCESSOR_PARAMS AccessorParams = {};

	ASSERT(module);
	if (!module) return status;
	Dispatcher = module->ModuleDispatcher;
	ASSERT(Dispatcher);

	AccessorParams.StdHeader.AltImageBasePtr = 0;
	AccessorParams.StdHeader.CalloutPtr = NULL;
	AccessorParams.StdHeader.Func = AMD_GET_VALUE;
	AccessorParams.StdHeader.ImageBasePtr = 0;

	AccessorParams.AllocationMethod = ByHost;
	AccessorParams.FieldName = name;
	AccessorParams.FieldValue = *value;
	AccessorParams.FieldSize = size;

	status = Dispatcher(&AccessorParams);
	ASSERT(AGESA_SUCCESS == status);

	*value = AccessorParams.FieldValue;
	size = AccessorParams.FieldSize;

	return status;
}
