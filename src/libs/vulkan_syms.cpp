#include <cassert>
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <vector>
#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>
#include "syms.hpp"
#include "trampoline.hpp"
#include "window.hpp"

typedef VkFlags VkAndroidSurfaceCreateFlagsKHR;

typedef struct VkAndroidSurfaceCreateInfoKHR {
    VkStructureType sType;
    const void* pNext;
    VkAndroidSurfaceCreateFlagsKHR flags;
    ANativeWindow* window;
} VkAndroidSurfaceCreateInfoKHR;

typedef VkResult (*PFN_vkCreateAndroidSurfaceKHR)(VkInstance,VkAndroidSurfaceCreateInfoKHR*,VkAllocationCallbacks*,VkSurfaceKHR*);

static std::unordered_map<std::string, usize> proc_to_trampoline;
static PFN_vkGetInstanceProcAddr proc_addr;
static PFN_vkGetDeviceProcAddr device_proc_addr;

static VkInstance global_instance;

static const std::vector<dynamic_symbol> actual_symbols = {
   { "vkAcquireDrmDisplayEXT", (PFN_vkAcquireDrmDisplayEXT)nullptr },
   { "vkAcquireNextImage2KHR", (PFN_vkAcquireNextImage2KHR)nullptr },
   { "vkAcquireNextImageKHR", (PFN_vkAcquireNextImageKHR)nullptr },
   { "vkAcquirePerformanceConfigurationINTEL", (PFN_vkAcquirePerformanceConfigurationINTEL)nullptr },
   { "vkAcquireProfilingLockKHR", (PFN_vkAcquireProfilingLockKHR)nullptr },
   { "vkAllocateCommandBuffers", (PFN_vkAllocateCommandBuffers)nullptr },
   { "vkAllocateDescriptorSets", (PFN_vkAllocateDescriptorSets)nullptr },
   { "vkAllocateMemory", (PFN_vkAllocateMemory)nullptr },
   { "vkBeginCommandBuffer", (PFN_vkBeginCommandBuffer)nullptr },
   { "vkBindAccelerationStructureMemoryNV", (PFN_vkBindAccelerationStructureMemoryNV)nullptr },
   { "vkBindBufferMemory2KHR", (PFN_vkBindBufferMemory2KHR)nullptr },
   { "vkBindBufferMemory2", (PFN_vkBindBufferMemory2)nullptr },
   { "vkBindBufferMemory", (PFN_vkBindBufferMemory)nullptr },
   { "vkBindImageMemory2KHR", (PFN_vkBindImageMemory2KHR)nullptr },
   { "vkBindImageMemory2", (PFN_vkBindImageMemory2)nullptr },
   { "vkBindImageMemory", (PFN_vkBindImageMemory)nullptr },
   { "vkBindOpticalFlowSessionImageNV", (PFN_vkBindOpticalFlowSessionImageNV)nullptr },
   { "vkBindVideoSessionMemoryKHR", (PFN_vkBindVideoSessionMemoryKHR)nullptr },
   { "vkBuildAccelerationStructuresKHR", (PFN_vkBuildAccelerationStructuresKHR)nullptr },
   { "vkBuildMicromapsEXT", (PFN_vkBuildMicromapsEXT)nullptr },
   { "vkCmdBeginConditionalRenderingEXT", (PFN_vkCmdBeginConditionalRenderingEXT)nullptr },
   { "vkCmdBeginDebugUtilsLabelEXT", (PFN_vkCmdBeginDebugUtilsLabelEXT)nullptr },
   { "vkCmdBeginQueryIndexedEXT", (PFN_vkCmdBeginQueryIndexedEXT)nullptr },
   { "vkCmdBeginQuery", (PFN_vkCmdBeginQuery)nullptr },
   { "vkCmdBeginRenderingKHR", (PFN_vkCmdBeginRenderingKHR)nullptr },
   { "vkCmdBeginRendering", (PFN_vkCmdBeginRendering)nullptr },
   { "vkCmdBeginRenderPass2KHR", (PFN_vkCmdBeginRenderPass2KHR)nullptr },
   { "vkCmdBeginRenderPass2", (PFN_vkCmdBeginRenderPass2)nullptr },
   { "vkCmdBeginRenderPass", (PFN_vkCmdBeginRenderPass)nullptr },
   { "vkCmdBeginTransformFeedbackEXT", (PFN_vkCmdBeginTransformFeedbackEXT)nullptr },
   { "vkCmdBeginVideoCodingKHR", (PFN_vkCmdBeginVideoCodingKHR)nullptr },
   { "vkCmdBindDescriptorBufferEmbeddedSamplersEXT", (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)nullptr },
   { "vkCmdBindDescriptorBuffersEXT", (PFN_vkCmdBindDescriptorBuffersEXT)nullptr },
   { "vkCmdBindDescriptorSets", (PFN_vkCmdBindDescriptorSets)nullptr },
   { "vkCmdBindIndexBuffer", (PFN_vkCmdBindIndexBuffer)nullptr },
   { "vkCmdBindInvocationMaskHUAWEI", (PFN_vkCmdBindInvocationMaskHUAWEI)nullptr },
   { "vkCmdBindPipelineShaderGroupNV", (PFN_vkCmdBindPipelineShaderGroupNV)nullptr },
   { "vkCmdBindPipeline", (PFN_vkCmdBindPipeline)nullptr },
   { "vkCmdBindShadersEXT", (PFN_vkCmdBindShadersEXT)nullptr },
   { "vkCmdBindShadingRateImageNV", (PFN_vkCmdBindShadingRateImageNV)nullptr },
   { "vkCmdBindTransformFeedbackBuffersEXT", (PFN_vkCmdBindTransformFeedbackBuffersEXT)nullptr },
   { "vkCmdBindVertexBuffers2EXT", (PFN_vkCmdBindVertexBuffers2EXT)nullptr },
   { "vkCmdBindVertexBuffers2", (PFN_vkCmdBindVertexBuffers2)nullptr },
   { "vkCmdBindVertexBuffers", (PFN_vkCmdBindVertexBuffers)nullptr },
   { "vkCmdBlitImage2KHR", (PFN_vkCmdBlitImage2KHR)nullptr },
   { "vkCmdBlitImage2", (PFN_vkCmdBlitImage2)nullptr },
   { "vkCmdBlitImage", (PFN_vkCmdBlitImage)nullptr },
   { "vkCmdBuildAccelerationStructureNV", (PFN_vkCmdBuildAccelerationStructureNV)nullptr },
   { "vkCmdBuildAccelerationStructuresIndirectKHR", (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)nullptr },
   { "vkCmdBuildAccelerationStructuresKHR", (PFN_vkCmdBuildAccelerationStructuresKHR)nullptr },
   { "vkCmdBuildMicromapsEXT", (PFN_vkCmdBuildMicromapsEXT)nullptr },
   { "vkCmdClearAttachments", (PFN_vkCmdClearAttachments)nullptr },
   { "vkCmdClearColorImage", (PFN_vkCmdClearColorImage)nullptr },
   { "vkCmdClearDepthStencilImage", (PFN_vkCmdClearDepthStencilImage)nullptr },
   { "vkCmdControlVideoCodingKHR", (PFN_vkCmdControlVideoCodingKHR)nullptr },
   { "vkCmdCopyAccelerationStructureKHR", (PFN_vkCmdCopyAccelerationStructureKHR)nullptr },
   { "vkCmdCopyAccelerationStructureNV", (PFN_vkCmdCopyAccelerationStructureNV)nullptr },
   { "vkCmdCopyAccelerationStructureToMemoryKHR", (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)nullptr },
   { "vkCmdCopyBuffer2KHR", (PFN_vkCmdCopyBuffer2KHR)nullptr },
   { "vkCmdCopyBuffer2", (PFN_vkCmdCopyBuffer2)nullptr },
   { "vkCmdCopyBufferToImage2KHR", (PFN_vkCmdCopyBufferToImage2KHR)nullptr },
   { "vkCmdCopyBufferToImage2", (PFN_vkCmdCopyBufferToImage2)nullptr },
   { "vkCmdCopyBufferToImage", (PFN_vkCmdCopyBufferToImage)nullptr },
   { "vkCmdCopyBuffer", (PFN_vkCmdCopyBuffer)nullptr },
   { "vkCmdCopyImage2KHR", (PFN_vkCmdCopyImage2KHR)nullptr },
   { "vkCmdCopyImage2", (PFN_vkCmdCopyImage2)nullptr },
   { "vkCmdCopyImageToBuffer2KHR", (PFN_vkCmdCopyImageToBuffer2KHR)nullptr },
   { "vkCmdCopyImageToBuffer2", (PFN_vkCmdCopyImageToBuffer2)nullptr },
   { "vkCmdCopyImageToBuffer", (PFN_vkCmdCopyImageToBuffer)nullptr },
   { "vkCmdCopyImage", (PFN_vkCmdCopyImage)nullptr },
   { "vkCmdCopyMemoryIndirectNV", (PFN_vkCmdCopyMemoryIndirectNV)nullptr },
   { "vkCmdCopyMemoryToAccelerationStructureKHR", (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)nullptr },
   { "vkCmdCopyMemoryToImageIndirectNV", (PFN_vkCmdCopyMemoryToImageIndirectNV)nullptr },
   { "vkCmdCopyMemoryToMicromapEXT", (PFN_vkCmdCopyMemoryToMicromapEXT)nullptr },
   { "vkCmdCopyMicromapEXT", (PFN_vkCmdCopyMicromapEXT)nullptr },
   { "vkCmdCopyMicromapToMemoryEXT", (PFN_vkCmdCopyMicromapToMemoryEXT)nullptr },
   { "vkCmdCopyQueryPoolResults", (PFN_vkCmdCopyQueryPoolResults)nullptr },
   { "vkCmdCuLaunchKernelNVX", (PFN_vkCmdCuLaunchKernelNVX)nullptr },
   { "vkCmdDebugMarkerBeginEXT", (PFN_vkCmdDebugMarkerBeginEXT)nullptr },
   { "vkCmdDebugMarkerEndEXT", (PFN_vkCmdDebugMarkerEndEXT)nullptr },
   { "vkCmdDebugMarkerInsertEXT", (PFN_vkCmdDebugMarkerInsertEXT)nullptr },
   { "vkCmdDecodeVideoKHR", (PFN_vkCmdDecodeVideoKHR)nullptr },
   { "vkCmdDecompressMemoryIndirectCountNV", (PFN_vkCmdDecompressMemoryIndirectCountNV)nullptr },
   { "vkCmdDecompressMemoryNV", (PFN_vkCmdDecompressMemoryNV)nullptr },
   { "vkCmdDispatchBaseKHR", (PFN_vkCmdDispatchBaseKHR)nullptr },
   { "vkCmdDispatchBase", (PFN_vkCmdDispatchBase)nullptr },
   { "vkCmdDispatchIndirect", (PFN_vkCmdDispatchIndirect)nullptr },
   { "vkCmdDispatch", (PFN_vkCmdDispatch)nullptr },
   { "vkCmdDrawClusterHUAWEI", (PFN_vkCmdDrawClusterHUAWEI)nullptr },
   { "vkCmdDrawClusterIndirectHUAWEI", (PFN_vkCmdDrawClusterIndirectHUAWEI)nullptr },
   { "vkCmdDrawIndexedIndirectCountAMD", (PFN_vkCmdDrawIndexedIndirectCountAMD)nullptr },
   { "vkCmdDrawIndexedIndirectCountKHR", (PFN_vkCmdDrawIndexedIndirectCountKHR)nullptr },
   { "vkCmdDrawIndexedIndirectCount", (PFN_vkCmdDrawIndexedIndirectCount)nullptr },
   { "vkCmdDrawIndexedIndirect", (PFN_vkCmdDrawIndexedIndirect)nullptr },
   { "vkCmdDrawIndexed", (PFN_vkCmdDrawIndexed)nullptr },
   { "vkCmdDrawIndirectByteCountEXT", (PFN_vkCmdDrawIndirectByteCountEXT)nullptr },
   { "vkCmdDrawIndirectCountAMD", (PFN_vkCmdDrawIndirectCountAMD)nullptr },
   { "vkCmdDrawIndirectCountKHR", (PFN_vkCmdDrawIndirectCountKHR)nullptr },
   { "vkCmdDrawIndirectCount", (PFN_vkCmdDrawIndirectCount)nullptr },
   { "vkCmdDrawIndirect", (PFN_vkCmdDrawIndirect)nullptr },
   { "vkCmdDrawMeshTasksEXT", (PFN_vkCmdDrawMeshTasksEXT)nullptr },
   { "vkCmdDrawMeshTasksIndirectCountEXT", (PFN_vkCmdDrawMeshTasksIndirectCountEXT)nullptr },
   { "vkCmdDrawMeshTasksIndirectCountNV", (PFN_vkCmdDrawMeshTasksIndirectCountNV)nullptr },
   { "vkCmdDrawMeshTasksIndirectEXT", (PFN_vkCmdDrawMeshTasksIndirectEXT)nullptr },
   { "vkCmdDrawMeshTasksIndirectNV", (PFN_vkCmdDrawMeshTasksIndirectNV)nullptr },
   { "vkCmdDrawMeshTasksNV", (PFN_vkCmdDrawMeshTasksNV)nullptr },
   { "vkCmdDrawMultiEXT", (PFN_vkCmdDrawMultiEXT)nullptr },
   { "vkCmdDrawMultiIndexedEXT", (PFN_vkCmdDrawMultiIndexedEXT)nullptr },
   { "vkCmdDraw", (PFN_vkCmdDraw)nullptr },
   { "vkCmdEndConditionalRenderingEXT", (PFN_vkCmdEndConditionalRenderingEXT)nullptr },
   { "vkCmdEndDebugUtilsLabelEXT", (PFN_vkCmdEndDebugUtilsLabelEXT)nullptr },
   { "vkCmdEndQueryIndexedEXT", (PFN_vkCmdEndQueryIndexedEXT)nullptr },
   { "vkCmdEndQuery", (PFN_vkCmdEndQuery)nullptr },
   { "vkCmdEndRenderingKHR", (PFN_vkCmdEndRenderingKHR)nullptr },
   { "vkCmdEndRendering", (PFN_vkCmdEndRendering)nullptr },
   { "vkCmdEndRenderPass2KHR", (PFN_vkCmdEndRenderPass2KHR)nullptr },
   { "vkCmdEndRenderPass2", (PFN_vkCmdEndRenderPass2)nullptr },
   { "vkCmdEndRenderPass", (PFN_vkCmdEndRenderPass)nullptr },
   { "vkCmdEndTransformFeedbackEXT", (PFN_vkCmdEndTransformFeedbackEXT)nullptr },
   { "vkCmdEndVideoCodingKHR", (PFN_vkCmdEndVideoCodingKHR)nullptr },
   { "vkCmdExecuteCommands", (PFN_vkCmdExecuteCommands)nullptr },
   { "vkCmdExecuteGeneratedCommandsNV", (PFN_vkCmdExecuteGeneratedCommandsNV)nullptr },
   { "vkCmdFillBuffer", (PFN_vkCmdFillBuffer)nullptr },
   { "vkCmdInsertDebugUtilsLabelEXT", (PFN_vkCmdInsertDebugUtilsLabelEXT)nullptr },
   { "vkCmdNextSubpass2KHR", (PFN_vkCmdNextSubpass2KHR)nullptr },
   { "vkCmdNextSubpass2", (PFN_vkCmdNextSubpass2)nullptr },
   { "vkCmdNextSubpass", (PFN_vkCmdNextSubpass)nullptr },
   { "vkCmdOpticalFlowExecuteNV", (PFN_vkCmdOpticalFlowExecuteNV)nullptr },
   { "vkCmdPipelineBarrier2KHR", (PFN_vkCmdPipelineBarrier2KHR)nullptr },
   { "vkCmdPipelineBarrier2", (PFN_vkCmdPipelineBarrier2)nullptr },
   { "vkCmdPipelineBarrier", (PFN_vkCmdPipelineBarrier)nullptr },
   { "vkCmdPreprocessGeneratedCommandsNV", (PFN_vkCmdPreprocessGeneratedCommandsNV)nullptr },
   { "vkCmdPushConstants", (PFN_vkCmdPushConstants)nullptr },
   { "vkCmdPushDescriptorSetKHR", (PFN_vkCmdPushDescriptorSetKHR)nullptr },
   { "vkCmdPushDescriptorSetWithTemplateKHR", (PFN_vkCmdPushDescriptorSetWithTemplateKHR)nullptr },
   { "vkCmdResetEvent2KHR", (PFN_vkCmdResetEvent2KHR)nullptr },
   { "vkCmdResetEvent2", (PFN_vkCmdResetEvent2)nullptr },
   { "vkCmdResetEvent", (PFN_vkCmdResetEvent)nullptr },
   { "vkCmdResetQueryPool", (PFN_vkCmdResetQueryPool)nullptr },
   { "vkCmdResolveImage2KHR", (PFN_vkCmdResolveImage2KHR)nullptr },
   { "vkCmdResolveImage2", (PFN_vkCmdResolveImage2)nullptr },
   { "vkCmdResolveImage", (PFN_vkCmdResolveImage)nullptr },
   { "vkCmdSetAlphaToCoverageEnableEXT", (PFN_vkCmdSetAlphaToCoverageEnableEXT)nullptr },
   { "vkCmdSetAlphaToOneEnableEXT", (PFN_vkCmdSetAlphaToOneEnableEXT)nullptr },
   { "vkCmdSetAttachmentFeedbackLoopEnableEXT", (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)nullptr },
   { "vkCmdSetBlendConstants", (PFN_vkCmdSetBlendConstants)nullptr },
   { "vkCmdSetCheckpointNV", (PFN_vkCmdSetCheckpointNV)nullptr },
   { "vkCmdSetCoarseSampleOrderNV", (PFN_vkCmdSetCoarseSampleOrderNV)nullptr },
   { "vkCmdSetColorBlendAdvancedEXT", (PFN_vkCmdSetColorBlendAdvancedEXT)nullptr },
   { "vkCmdSetColorBlendEnableEXT", (PFN_vkCmdSetColorBlendEnableEXT)nullptr },
   { "vkCmdSetColorBlendEquationEXT", (PFN_vkCmdSetColorBlendEquationEXT)nullptr },
   { "vkCmdSetColorWriteEnableEXT", (PFN_vkCmdSetColorWriteEnableEXT)nullptr },
   { "vkCmdSetColorWriteMaskEXT", (PFN_vkCmdSetColorWriteMaskEXT)nullptr },
   { "vkCmdSetConservativeRasterizationModeEXT", (PFN_vkCmdSetConservativeRasterizationModeEXT)nullptr },
   { "vkCmdSetCoverageModulationModeNV", (PFN_vkCmdSetCoverageModulationModeNV)nullptr },
   { "vkCmdSetCoverageModulationTableEnableNV", (PFN_vkCmdSetCoverageModulationTableEnableNV)nullptr },
   { "vkCmdSetCoverageModulationTableNV", (PFN_vkCmdSetCoverageModulationTableNV)nullptr },
   { "vkCmdSetCoverageReductionModeNV", (PFN_vkCmdSetCoverageReductionModeNV)nullptr },
   { "vkCmdSetCoverageToColorEnableNV", (PFN_vkCmdSetCoverageToColorEnableNV)nullptr },
   { "vkCmdSetCoverageToColorLocationNV", (PFN_vkCmdSetCoverageToColorLocationNV)nullptr },
   { "vkCmdSetCullModeEXT", (PFN_vkCmdSetCullModeEXT)nullptr },
   { "vkCmdSetCullMode", (PFN_vkCmdSetCullMode)nullptr },
   { "vkCmdSetDepthBiasEnableEXT", (PFN_vkCmdSetDepthBiasEnableEXT)nullptr },
   { "vkCmdSetDepthBiasEnable", (PFN_vkCmdSetDepthBiasEnable)nullptr },
   { "vkCmdSetDepthBias", (PFN_vkCmdSetDepthBias)nullptr },
   { "vkCmdSetDepthBoundsTestEnableEXT", (PFN_vkCmdSetDepthBoundsTestEnableEXT)nullptr },
   { "vkCmdSetDepthBoundsTestEnable", (PFN_vkCmdSetDepthBoundsTestEnable)nullptr },
   { "vkCmdSetDepthBounds", (PFN_vkCmdSetDepthBounds)nullptr },
   { "vkCmdSetDepthClampEnableEXT", (PFN_vkCmdSetDepthClampEnableEXT)nullptr },
   { "vkCmdSetDepthClipEnableEXT", (PFN_vkCmdSetDepthClipEnableEXT)nullptr },
   { "vkCmdSetDepthClipNegativeOneToOneEXT", (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)nullptr },
   { "vkCmdSetDepthCompareOpEXT", (PFN_vkCmdSetDepthCompareOpEXT)nullptr },
   { "vkCmdSetDepthCompareOp", (PFN_vkCmdSetDepthCompareOp)nullptr },
   { "vkCmdSetDepthTestEnableEXT", (PFN_vkCmdSetDepthTestEnableEXT)nullptr },
   { "vkCmdSetDepthTestEnable", (PFN_vkCmdSetDepthTestEnable)nullptr },
   { "vkCmdSetDepthWriteEnableEXT", (PFN_vkCmdSetDepthWriteEnableEXT)nullptr },
   { "vkCmdSetDepthWriteEnable", (PFN_vkCmdSetDepthWriteEnable)nullptr },
   { "vkCmdSetDescriptorBufferOffsetsEXT", (PFN_vkCmdSetDescriptorBufferOffsetsEXT)nullptr },
   { "vkCmdSetDeviceMaskKHR", (PFN_vkCmdSetDeviceMaskKHR)nullptr },
   { "vkCmdSetDeviceMask", (PFN_vkCmdSetDeviceMask)nullptr },
   { "vkCmdSetDiscardRectangleEnableEXT", (PFN_vkCmdSetDiscardRectangleEnableEXT)nullptr },
   { "vkCmdSetDiscardRectangleEXT", (PFN_vkCmdSetDiscardRectangleEXT)nullptr },
   { "vkCmdSetDiscardRectangleModeEXT", (PFN_vkCmdSetDiscardRectangleModeEXT)nullptr },
   { "vkCmdSetEvent2KHR", (PFN_vkCmdSetEvent2KHR)nullptr },
   { "vkCmdSetEvent2", (PFN_vkCmdSetEvent2)nullptr },
   { "vkCmdSetEvent", (PFN_vkCmdSetEvent)nullptr },
   { "vkCmdSetExclusiveScissorEnableNV", (PFN_vkCmdSetExclusiveScissorEnableNV)nullptr },
   { "vkCmdSetExclusiveScissorNV", (PFN_vkCmdSetExclusiveScissorNV)nullptr },
   { "vkCmdSetExtraPrimitiveOverestimationSizeEXT", (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)nullptr },
   { "vkCmdSetFragmentShadingRateEnumNV", (PFN_vkCmdSetFragmentShadingRateEnumNV)nullptr },
   { "vkCmdSetFragmentShadingRateKHR", (PFN_vkCmdSetFragmentShadingRateKHR)nullptr },
   { "vkCmdSetFrontFaceEXT", (PFN_vkCmdSetFrontFaceEXT)nullptr },
   { "vkCmdSetFrontFace", (PFN_vkCmdSetFrontFace)nullptr },
   { "vkCmdSetLineRasterizationModeEXT", (PFN_vkCmdSetLineRasterizationModeEXT)nullptr },
   { "vkCmdSetLineStippleEnableEXT", (PFN_vkCmdSetLineStippleEnableEXT)nullptr },
   { "vkCmdSetLineStippleEXT", (PFN_vkCmdSetLineStippleEXT)nullptr },
   { "vkCmdSetLineWidth", (PFN_vkCmdSetLineWidth)nullptr },
   { "vkCmdSetLogicOpEnableEXT", (PFN_vkCmdSetLogicOpEnableEXT)nullptr },
   { "vkCmdSetLogicOpEXT", (PFN_vkCmdSetLogicOpEXT)nullptr },
   { "vkCmdSetPatchControlPointsEXT", (PFN_vkCmdSetPatchControlPointsEXT)nullptr },
   { "vkCmdSetPerformanceMarkerINTEL", (PFN_vkCmdSetPerformanceMarkerINTEL)nullptr },
   { "vkCmdSetPerformanceOverrideINTEL", (PFN_vkCmdSetPerformanceOverrideINTEL)nullptr },
   { "vkCmdSetPerformanceStreamMarkerINTEL", (PFN_vkCmdSetPerformanceStreamMarkerINTEL)nullptr },
   { "vkCmdSetPolygonModeEXT", (PFN_vkCmdSetPolygonModeEXT)nullptr },
   { "vkCmdSetPrimitiveRestartEnableEXT", (PFN_vkCmdSetPrimitiveRestartEnableEXT)nullptr },
   { "vkCmdSetPrimitiveRestartEnable", (PFN_vkCmdSetPrimitiveRestartEnable)nullptr },
   { "vkCmdSetPrimitiveTopologyEXT", (PFN_vkCmdSetPrimitiveTopologyEXT)nullptr },
   { "vkCmdSetPrimitiveTopology", (PFN_vkCmdSetPrimitiveTopology)nullptr },
   { "vkCmdSetProvokingVertexModeEXT", (PFN_vkCmdSetProvokingVertexModeEXT)nullptr },
   { "vkCmdSetRasterizationSamplesEXT", (PFN_vkCmdSetRasterizationSamplesEXT)nullptr },
   { "vkCmdSetRasterizationStreamEXT", (PFN_vkCmdSetRasterizationStreamEXT)nullptr },
   { "vkCmdSetRasterizerDiscardEnableEXT", (PFN_vkCmdSetRasterizerDiscardEnableEXT)nullptr },
   { "vkCmdSetRasterizerDiscardEnable", (PFN_vkCmdSetRasterizerDiscardEnable)nullptr },
   { "vkCmdSetRayTracingPipelineStackSizeKHR", (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)nullptr },
   { "vkCmdSetRepresentativeFragmentTestEnableNV", (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)nullptr },
   { "vkCmdSetSampleLocationsEnableEXT", (PFN_vkCmdSetSampleLocationsEnableEXT)nullptr },
   { "vkCmdSetSampleLocationsEXT", (PFN_vkCmdSetSampleLocationsEXT)nullptr },
   { "vkCmdSetSampleMaskEXT", (PFN_vkCmdSetSampleMaskEXT)nullptr },
   { "vkCmdSetScissor", (PFN_vkCmdSetScissor)nullptr },
   { "vkCmdSetScissorWithCountEXT", (PFN_vkCmdSetScissorWithCountEXT)nullptr },
   { "vkCmdSetScissorWithCount", (PFN_vkCmdSetScissorWithCount)nullptr },
   { "vkCmdSetShadingRateImageEnableNV", (PFN_vkCmdSetShadingRateImageEnableNV)nullptr },
   { "vkCmdSetStencilCompareMask", (PFN_vkCmdSetStencilCompareMask)nullptr },
   { "vkCmdSetStencilOpEXT", (PFN_vkCmdSetStencilOpEXT)nullptr },
   { "vkCmdSetStencilOp", (PFN_vkCmdSetStencilOp)nullptr },
   { "vkCmdSetStencilReference", (PFN_vkCmdSetStencilReference)nullptr },
   { "vkCmdSetStencilTestEnableEXT", (PFN_vkCmdSetStencilTestEnableEXT)nullptr },
   { "vkCmdSetStencilTestEnable", (PFN_vkCmdSetStencilTestEnable)nullptr },
   { "vkCmdSetStencilWriteMask", (PFN_vkCmdSetStencilWriteMask)nullptr },
   { "vkCmdSetTessellationDomainOriginEXT", (PFN_vkCmdSetTessellationDomainOriginEXT)nullptr },
   { "vkCmdSetVertexInputEXT", (PFN_vkCmdSetVertexInputEXT)nullptr },
   { "vkCmdSetViewportShadingRatePaletteNV", (PFN_vkCmdSetViewportShadingRatePaletteNV)nullptr },
   { "vkCmdSetViewportSwizzleNV", (PFN_vkCmdSetViewportSwizzleNV)nullptr },
   { "vkCmdSetViewport", (PFN_vkCmdSetViewport)nullptr },
   { "vkCmdSetViewportWithCountEXT", (PFN_vkCmdSetViewportWithCountEXT)nullptr },
   { "vkCmdSetViewportWithCount", (PFN_vkCmdSetViewportWithCount)nullptr },
   { "vkCmdSetViewportWScalingEnableNV", (PFN_vkCmdSetViewportWScalingEnableNV)nullptr },
   { "vkCmdSetViewportWScalingNV", (PFN_vkCmdSetViewportWScalingNV)nullptr },
   { "vkCmdSubpassShadingHUAWEI", (PFN_vkCmdSubpassShadingHUAWEI)nullptr },
   { "vkCmdTraceRaysIndirect2KHR", (PFN_vkCmdTraceRaysIndirect2KHR)nullptr },
   { "vkCmdTraceRaysIndirectKHR", (PFN_vkCmdTraceRaysIndirectKHR)nullptr },
   { "vkCmdTraceRaysKHR", (PFN_vkCmdTraceRaysKHR)nullptr },
   { "vkCmdTraceRaysNV", (PFN_vkCmdTraceRaysNV)nullptr },
   { "vkCmdUpdateBuffer", (PFN_vkCmdUpdateBuffer)nullptr },
   { "vkCmdWaitEvents2KHR", (PFN_vkCmdWaitEvents2KHR)nullptr },
   { "vkCmdWaitEvents2", (PFN_vkCmdWaitEvents2)nullptr },
   { "vkCmdWaitEvents", (PFN_vkCmdWaitEvents)nullptr },
   { "vkCmdWriteAccelerationStructuresPropertiesKHR", (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)nullptr },
   { "vkCmdWriteAccelerationStructuresPropertiesNV", (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)nullptr },
   { "vkCmdWriteBufferMarker2AMD", (PFN_vkCmdWriteBufferMarker2AMD)nullptr },
   { "vkCmdWriteBufferMarkerAMD", (PFN_vkCmdWriteBufferMarkerAMD)nullptr },
   { "vkCmdWriteMicromapsPropertiesEXT", (PFN_vkCmdWriteMicromapsPropertiesEXT)nullptr },
   { "vkCmdWriteTimestamp2KHR", (PFN_vkCmdWriteTimestamp2KHR)nullptr },
   { "vkCmdWriteTimestamp2", (PFN_vkCmdWriteTimestamp2)nullptr },
   { "vkCmdWriteTimestamp", (PFN_vkCmdWriteTimestamp)nullptr },
   { "vkCompileDeferredNV", (PFN_vkCompileDeferredNV)nullptr },
   { "vkCopyAccelerationStructureKHR", (PFN_vkCopyAccelerationStructureKHR)nullptr },
   { "vkCopyAccelerationStructureToMemoryKHR", (PFN_vkCopyAccelerationStructureToMemoryKHR)nullptr },
   { "vkCopyMemoryToAccelerationStructureKHR", (PFN_vkCopyMemoryToAccelerationStructureKHR)nullptr },
   { "vkCopyMemoryToMicromapEXT", (PFN_vkCopyMemoryToMicromapEXT)nullptr },
   { "vkCopyMicromapEXT", (PFN_vkCopyMicromapEXT)nullptr },
   { "vkCopyMicromapToMemoryEXT", (PFN_vkCopyMicromapToMemoryEXT)nullptr },
   { "vkCreateAccelerationStructureKHR", (PFN_vkCreateAccelerationStructureKHR)nullptr },
   { "vkCreateAccelerationStructureNV", (PFN_vkCreateAccelerationStructureNV)nullptr },
   { "vkCreateAndroidSurfaceKHR", (PFN_vkCreateAndroidSurfaceKHR)nullptr},
   { "vkCreateBufferView", (PFN_vkCreateBufferView)nullptr },
   { "vkCreateBuffer", (PFN_vkCreateBuffer)nullptr },
   { "vkCreateCommandPool", (PFN_vkCreateCommandPool)nullptr },
   { "vkCreateComputePipelines", (PFN_vkCreateComputePipelines)nullptr },
   { "vkCreateCuFunctionNVX", (PFN_vkCreateCuFunctionNVX)nullptr },
   { "vkCreateCuModuleNVX", (PFN_vkCreateCuModuleNVX)nullptr },
   { "vkCreateDebugReportCallbackEXT", (PFN_vkCreateDebugReportCallbackEXT)nullptr },
   { "vkCreateDebugUtilsMessengerEXT", (PFN_vkCreateDebugUtilsMessengerEXT)nullptr },
   { "vkCreateDeferredOperationKHR", (PFN_vkCreateDeferredOperationKHR)nullptr },
   { "vkCreateDescriptorPool", (PFN_vkCreateDescriptorPool)nullptr },
   { "vkCreateDescriptorSetLayout", (PFN_vkCreateDescriptorSetLayout)nullptr },
   { "vkCreateDescriptorUpdateTemplateKHR", (PFN_vkCreateDescriptorUpdateTemplateKHR)nullptr },
   { "vkCreateDescriptorUpdateTemplate", (PFN_vkCreateDescriptorUpdateTemplate)nullptr },
   { "vkCreateDevice", (PFN_vkCreateDevice)nullptr },
   { "vkCreateDisplayModeKHR", (PFN_vkCreateDisplayModeKHR)nullptr },
   { "vkCreateDisplayPlaneSurfaceKHR", (PFN_vkCreateDisplayPlaneSurfaceKHR)nullptr },
   { "vkCreateEvent", (PFN_vkCreateEvent)nullptr },
   { "vkCreateFence", (PFN_vkCreateFence)nullptr },
   { "vkCreateFramebuffer", (PFN_vkCreateFramebuffer)nullptr },
   { "vkCreateGraphicsPipelines", (PFN_vkCreateGraphicsPipelines)nullptr },
   { "vkCreateHeadlessSurfaceEXT", (PFN_vkCreateHeadlessSurfaceEXT)nullptr },
   { "vkCreateImageView", (PFN_vkCreateImageView)nullptr },
   { "vkCreateImage", (PFN_vkCreateImage)nullptr },
   { "vkCreateIndirectCommandsLayoutNV", (PFN_vkCreateIndirectCommandsLayoutNV)nullptr },
   { "vkCreateInstance", (PFN_vkCreateInstance)nullptr},
   { "vkCreateMicromapEXT", (PFN_vkCreateMicromapEXT)nullptr },
   { "vkCreateOpticalFlowSessionNV", (PFN_vkCreateOpticalFlowSessionNV)nullptr },
   { "vkCreatePipelineCache", (PFN_vkCreatePipelineCache)nullptr },
   { "vkCreatePipelineLayout", (PFN_vkCreatePipelineLayout)nullptr },
   { "vkCreatePrivateDataSlotEXT", (PFN_vkCreatePrivateDataSlotEXT)nullptr },
   { "vkCreatePrivateDataSlot", (PFN_vkCreatePrivateDataSlot)nullptr },
   { "vkCreateQueryPool", (PFN_vkCreateQueryPool)nullptr },
   { "vkCreateRayTracingPipelinesKHR", (PFN_vkCreateRayTracingPipelinesKHR)nullptr },
   { "vkCreateRayTracingPipelinesNV", (PFN_vkCreateRayTracingPipelinesNV)nullptr },
   { "vkCreateRenderPass2KHR", (PFN_vkCreateRenderPass2KHR)nullptr },
   { "vkCreateRenderPass2", (PFN_vkCreateRenderPass2)nullptr },
   { "vkCreateRenderPass", (PFN_vkCreateRenderPass)nullptr },
   { "vkCreateSampler", (PFN_vkCreateSampler)nullptr },
   { "vkCreateSamplerYcbcrConversionKHR", (PFN_vkCreateSamplerYcbcrConversionKHR)nullptr },
   { "vkCreateSamplerYcbcrConversion", (PFN_vkCreateSamplerYcbcrConversion)nullptr },
   { "vkCreateSemaphore", (PFN_vkCreateSemaphore)nullptr },
   { "vkCreateShaderModule", (PFN_vkCreateShaderModule)nullptr },
   { "vkCreateShadersEXT", (PFN_vkCreateShadersEXT)nullptr },
   { "vkCreateSharedSwapchainsKHR", (PFN_vkCreateSharedSwapchainsKHR)nullptr },
   { "vkCreateSwapchainKHR", (PFN_vkCreateSwapchainKHR)nullptr },
   { "vkCreateValidationCacheEXT", (PFN_vkCreateValidationCacheEXT)nullptr },
   { "vkCreateVideoSessionKHR", (PFN_vkCreateVideoSessionKHR)nullptr },
   { "vkCreateVideoSessionParametersKHR", (PFN_vkCreateVideoSessionParametersKHR)nullptr },
   { "vkDebugMarkerSetObjectNameEXT", (PFN_vkDebugMarkerSetObjectNameEXT)nullptr },
   { "vkDebugMarkerSetObjectTagEXT", (PFN_vkDebugMarkerSetObjectTagEXT)nullptr },
   { "vkDebugReportMessageEXT", (PFN_vkDebugReportMessageEXT)nullptr },
   { "vkDeferredOperationJoinKHR", (PFN_vkDeferredOperationJoinKHR)nullptr },
   { "vkDestroyAccelerationStructureKHR", (PFN_vkDestroyAccelerationStructureKHR)nullptr },
   { "vkDestroyAccelerationStructureNV", (PFN_vkDestroyAccelerationStructureNV)nullptr },
   { "vkDestroyBufferView", (PFN_vkDestroyBufferView)nullptr },
   { "vkDestroyBuffer", (PFN_vkDestroyBuffer)nullptr },
   { "vkDestroyCommandPool", (PFN_vkDestroyCommandPool)nullptr },
   { "vkDestroyCuFunctionNVX", (PFN_vkDestroyCuFunctionNVX)nullptr },
   { "vkDestroyCuModuleNVX", (PFN_vkDestroyCuModuleNVX)nullptr },
   { "vkDestroyDebugReportCallbackEXT", (PFN_vkDestroyDebugReportCallbackEXT)nullptr },
   { "vkDestroyDebugUtilsMessengerEXT", (PFN_vkDestroyDebugUtilsMessengerEXT)nullptr },
   { "vkDestroyDeferredOperationKHR", (PFN_vkDestroyDeferredOperationKHR)nullptr },
   { "vkDestroyDescriptorPool", (PFN_vkDestroyDescriptorPool)nullptr },
   { "vkDestroyDescriptorSetLayout", (PFN_vkDestroyDescriptorSetLayout)nullptr },
   { "vkDestroyDescriptorUpdateTemplateKHR", (PFN_vkDestroyDescriptorUpdateTemplateKHR)nullptr },
   { "vkDestroyDescriptorUpdateTemplate", (PFN_vkDestroyDescriptorUpdateTemplate)nullptr },
   { "vkDestroyDevice", (PFN_vkDestroyDevice)nullptr },
   { "vkDestroyEvent", (PFN_vkDestroyEvent)nullptr },
   { "vkDestroyFence", (PFN_vkDestroyFence)nullptr },
   { "vkDestroyFramebuffer", (PFN_vkDestroyFramebuffer)nullptr },
   { "vkDestroyImageView", (PFN_vkDestroyImageView)nullptr },
   { "vkDestroyImage", (PFN_vkDestroyImage)nullptr },
   { "vkDestroyIndirectCommandsLayoutNV", (PFN_vkDestroyIndirectCommandsLayoutNV)nullptr },
   { "vkDestroyInstance", (PFN_vkDestroyInstance)nullptr },
   { "vkDestroyMicromapEXT", (PFN_vkDestroyMicromapEXT)nullptr },
   { "vkDestroyOpticalFlowSessionNV", (PFN_vkDestroyOpticalFlowSessionNV)nullptr },
   { "vkDestroyPipelineCache", (PFN_vkDestroyPipelineCache)nullptr },
   { "vkDestroyPipelineLayout", (PFN_vkDestroyPipelineLayout)nullptr },
   { "vkDestroyPipeline", (PFN_vkDestroyPipeline)nullptr },
   { "vkDestroyPrivateDataSlotEXT", (PFN_vkDestroyPrivateDataSlotEXT)nullptr },
   { "vkDestroyPrivateDataSlot", (PFN_vkDestroyPrivateDataSlot)nullptr },
   { "vkDestroyQueryPool", (PFN_vkDestroyQueryPool)nullptr },
   { "vkDestroyRenderPass", (PFN_vkDestroyRenderPass)nullptr },
   { "vkDestroySampler", (PFN_vkDestroySampler)nullptr },
   { "vkDestroySamplerYcbcrConversionKHR", (PFN_vkDestroySamplerYcbcrConversionKHR)nullptr },
   { "vkDestroySamplerYcbcrConversion", (PFN_vkDestroySamplerYcbcrConversion)nullptr },
   { "vkDestroySemaphore", (PFN_vkDestroySemaphore)nullptr },
   { "vkDestroyShaderEXT", (PFN_vkDestroyShaderEXT)nullptr },
   { "vkDestroyShaderModule", (PFN_vkDestroyShaderModule)nullptr },
   { "vkDestroySurfaceKHR", (PFN_vkDestroySurfaceKHR)nullptr },
   { "vkDestroySwapchainKHR", (PFN_vkDestroySwapchainKHR)nullptr },
   { "vkDestroyValidationCacheEXT", (PFN_vkDestroyValidationCacheEXT)nullptr },
   { "vkDestroyVideoSessionKHR", (PFN_vkDestroyVideoSessionKHR)nullptr },
   { "vkDestroyVideoSessionParametersKHR", (PFN_vkDestroyVideoSessionParametersKHR)nullptr },
   { "vkDeviceWaitIdle", (PFN_vkDeviceWaitIdle)nullptr },
   { "vkDisplayPowerControlEXT", (PFN_vkDisplayPowerControlEXT)nullptr },
   { "vkEndCommandBuffer", (PFN_vkEndCommandBuffer)nullptr },
   { "vkEnumerateDeviceExtensionProperties", (PFN_vkEnumerateDeviceExtensionProperties)nullptr },
   { "vkEnumerateDeviceLayerProperties", (PFN_vkEnumerateDeviceLayerProperties)nullptr },
   { "vkEnumerateInstanceExtensionProperties", (PFN_vkEnumerateInstanceExtensionProperties)nullptr },
   { "vkEnumerateInstanceLayerProperties", (PFN_vkEnumerateInstanceLayerProperties)nullptr },
   { "vkEnumerateInstanceVersion", (PFN_vkEnumerateInstanceVersion)nullptr },
   { "vkEnumeratePhysicalDeviceGroupsKHR", (PFN_vkEnumeratePhysicalDeviceGroupsKHR)nullptr },
   { "vkEnumeratePhysicalDeviceGroups", (PFN_vkEnumeratePhysicalDeviceGroups)nullptr },
   { "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR", (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)nullptr },
   { "vkEnumeratePhysicalDevices", (PFN_vkEnumeratePhysicalDevices)nullptr },
   { "vkFlushMappedMemoryRanges", (PFN_vkFlushMappedMemoryRanges)nullptr },
   { "vkFreeCommandBuffers", (PFN_vkFreeCommandBuffers)nullptr },
   { "vkFreeDescriptorSets", (PFN_vkFreeDescriptorSets)nullptr },
   { "vkFreeMemory", (PFN_vkFreeMemory)nullptr },
   { "vkGetAccelerationStructureBuildSizesKHR", (PFN_vkGetAccelerationStructureBuildSizesKHR)nullptr },
   { "vkGetAccelerationStructureDeviceAddressKHR", (PFN_vkGetAccelerationStructureDeviceAddressKHR)nullptr },
   { "vkGetAccelerationStructureHandleNV", (PFN_vkGetAccelerationStructureHandleNV)nullptr },
   { "vkGetAccelerationStructureMemoryRequirementsNV", (PFN_vkGetAccelerationStructureMemoryRequirementsNV)nullptr },
   { "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT", (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)nullptr },
   { "vkGetAndroidHardwareBufferPropertiesANDROID", (PFN_vkVoidFunction)nullptr },
   { "vkGetBufferDeviceAddressEXT", (PFN_vkGetBufferDeviceAddressEXT)nullptr },
   { "vkGetBufferDeviceAddressKHR", (PFN_vkGetBufferDeviceAddressKHR)nullptr },
   { "vkGetBufferDeviceAddress", (PFN_vkGetBufferDeviceAddress)nullptr },
   { "vkGetBufferMemoryRequirements2KHR", (PFN_vkGetBufferMemoryRequirements2KHR)nullptr },
   { "vkGetBufferMemoryRequirements2", (PFN_vkGetBufferMemoryRequirements2)nullptr },
   { "vkGetBufferMemoryRequirements", (PFN_vkGetBufferMemoryRequirements)nullptr },
   { "vkGetBufferOpaqueCaptureAddressKHR", (PFN_vkGetBufferOpaqueCaptureAddressKHR)nullptr },
   { "vkGetBufferOpaqueCaptureAddress", (PFN_vkGetBufferOpaqueCaptureAddress)nullptr },
   { "vkGetBufferOpaqueCaptureDescriptorDataEXT", (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)nullptr },
   { "vkGetCalibratedTimestampsEXT", (PFN_vkGetCalibratedTimestampsEXT)nullptr },
   { "vkGetDeferredOperationMaxConcurrencyKHR", (PFN_vkGetDeferredOperationMaxConcurrencyKHR)nullptr },
   { "vkGetDeferredOperationResultKHR", (PFN_vkGetDeferredOperationResultKHR)nullptr },
   { "vkGetDescriptorEXT", (PFN_vkGetDescriptorEXT)nullptr },
   { "vkGetDescriptorSetHostMappingVALVE", (PFN_vkGetDescriptorSetHostMappingVALVE)nullptr },
   { "vkGetDescriptorSetLayoutBindingOffsetEXT", (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)nullptr },
   { "vkGetDescriptorSetLayoutHostMappingInfoVALVE", (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)nullptr },
   { "vkGetDescriptorSetLayoutSizeEXT", (PFN_vkGetDescriptorSetLayoutSizeEXT)nullptr },
   { "vkGetDescriptorSetLayoutSupportKHR", (PFN_vkGetDescriptorSetLayoutSupportKHR)nullptr },
   { "vkGetDescriptorSetLayoutSupport", (PFN_vkGetDescriptorSetLayoutSupport)nullptr },
   { "vkGetDeviceAccelerationStructureCompatibilityKHR", (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)nullptr },
   { "vkGetDeviceBufferMemoryRequirementsKHR", (PFN_vkGetDeviceBufferMemoryRequirementsKHR)nullptr },
   { "vkGetDeviceBufferMemoryRequirements", (PFN_vkGetDeviceBufferMemoryRequirements)nullptr },
   { "vkGetDeviceFaultInfoEXT", (PFN_vkGetDeviceFaultInfoEXT)nullptr },
   { "vkGetDeviceGroupPeerMemoryFeaturesKHR", (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)nullptr },
   { "vkGetDeviceGroupPeerMemoryFeatures", (PFN_vkGetDeviceGroupPeerMemoryFeatures)nullptr },
   { "vkGetDeviceGroupPresentCapabilitiesKHR", (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)nullptr },
   { "vkGetDeviceGroupSurfacePresentModesKHR", (PFN_vkGetDeviceGroupSurfacePresentModesKHR)nullptr },
   { "vkGetDeviceImageMemoryRequirementsKHR", (PFN_vkGetDeviceImageMemoryRequirementsKHR)nullptr },
   { "vkGetDeviceImageMemoryRequirements", (PFN_vkGetDeviceImageMemoryRequirements)nullptr },
   { "vkGetDeviceImageSparseMemoryRequirementsKHR", (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR)nullptr },
   { "vkGetDeviceImageSparseMemoryRequirements", (PFN_vkGetDeviceImageSparseMemoryRequirements)nullptr },
   { "vkGetDeviceMemoryCommitment", (PFN_vkGetDeviceMemoryCommitment)nullptr },
   { "vkGetDeviceMemoryOpaqueCaptureAddressKHR", (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)nullptr },
   { "vkGetDeviceMemoryOpaqueCaptureAddress", (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)nullptr },
   { "vkGetDeviceMicromapCompatibilityEXT", (PFN_vkGetDeviceMicromapCompatibilityEXT)nullptr },
   { "vkGetDeviceProcAddr", (PFN_vkGetDeviceProcAddr)nullptr},
   { "vkGetDeviceQueue2", (PFN_vkGetDeviceQueue2)nullptr },
   { "vkGetDeviceQueue", (PFN_vkGetDeviceQueue)nullptr },
   { "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI", (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)nullptr },
   { "vkGetDisplayModeProperties2KHR", (PFN_vkGetDisplayModeProperties2KHR)nullptr },
   { "vkGetDisplayModePropertiesKHR", (PFN_vkGetDisplayModePropertiesKHR)nullptr },
   { "vkGetDisplayPlaneCapabilities2KHR", (PFN_vkGetDisplayPlaneCapabilities2KHR)nullptr },
   { "vkGetDisplayPlaneCapabilitiesKHR", (PFN_vkGetDisplayPlaneCapabilitiesKHR)nullptr },
   { "vkGetDisplayPlaneSupportedDisplaysKHR", (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)nullptr },
   { "vkGetDrmDisplayEXT", (PFN_vkGetDrmDisplayEXT)nullptr },
   { "vkGetDynamicRenderingTilePropertiesQCOM", (PFN_vkGetDynamicRenderingTilePropertiesQCOM)nullptr },
   { "vkGetEventStatus", (PFN_vkGetEventStatus)nullptr },
   { "vkGetFenceFdKHR", (PFN_vkGetFenceFdKHR)nullptr },
   { "vkGetFenceStatus", (PFN_vkGetFenceStatus)nullptr },
   { "vkGetFramebufferTilePropertiesQCOM", (PFN_vkGetFramebufferTilePropertiesQCOM)nullptr },
   { "vkGetGeneratedCommandsMemoryRequirementsNV", (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)nullptr },
   { "vkGetImageDrmFormatModifierPropertiesEXT", (PFN_vkGetImageDrmFormatModifierPropertiesEXT)nullptr },
   { "vkGetImageMemoryRequirements2KHR", (PFN_vkGetImageMemoryRequirements2KHR)nullptr },
   { "vkGetImageMemoryRequirements2", (PFN_vkGetImageMemoryRequirements2)nullptr },
   { "vkGetImageMemoryRequirements", (PFN_vkGetImageMemoryRequirements)nullptr },
   { "vkGetImageOpaqueCaptureDescriptorDataEXT", (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)nullptr },
   { "vkGetImageSparseMemoryRequirements2KHR", (PFN_vkGetImageSparseMemoryRequirements2KHR)nullptr },
   { "vkGetImageSparseMemoryRequirements2", (PFN_vkGetImageSparseMemoryRequirements2)nullptr },
   { "vkGetImageSparseMemoryRequirements", (PFN_vkGetImageSparseMemoryRequirements)nullptr },
   { "vkGetImageSubresourceLayout2EXT", (PFN_vkGetImageSubresourceLayout2EXT)nullptr },
   { "vkGetImageSubresourceLayout", (PFN_vkGetImageSubresourceLayout)nullptr },
   { "vkGetImageViewAddressNVX", (PFN_vkGetImageViewAddressNVX)nullptr },
   { "vkGetImageViewHandleNVX", (PFN_vkGetImageViewHandleNVX)nullptr },
   { "vkGetImageViewOpaqueCaptureDescriptorDataEXT", (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)nullptr },
   { "vkGetMemoryAndroidHardwareBufferANDROID", (PFN_vkVoidFunction)nullptr },
   { "vkGetMemoryFdKHR", (PFN_vkGetMemoryFdKHR)nullptr },
   { "vkGetMemoryFdPropertiesKHR", (PFN_vkGetMemoryFdPropertiesKHR)nullptr },
   { "vkGetMemoryHostPointerPropertiesEXT", (PFN_vkGetMemoryHostPointerPropertiesEXT)nullptr },
   { "vkGetMemoryRemoteAddressNV", (PFN_vkGetMemoryRemoteAddressNV)nullptr },
   { "vkGetMicromapBuildSizesEXT", (PFN_vkGetMicromapBuildSizesEXT)nullptr },
   { "vkGetPastPresentationTimingGOOGLE", (PFN_vkGetPastPresentationTimingGOOGLE)nullptr },
   { "vkGetPerformanceParameterINTEL", (PFN_vkGetPerformanceParameterINTEL)nullptr },
   { "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT", (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)nullptr },
   { "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV", (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)nullptr },
   { "vkGetPhysicalDeviceDisplayPlaneProperties2KHR", (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)nullptr },
   { "vkGetPhysicalDeviceDisplayPlanePropertiesKHR", (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)nullptr },
   { "vkGetPhysicalDeviceDisplayProperties2KHR", (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)nullptr },
   { "vkGetPhysicalDeviceDisplayPropertiesKHR", (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)nullptr },
   { "vkGetPhysicalDeviceExternalBufferPropertiesKHR", (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)nullptr },
   { "vkGetPhysicalDeviceExternalBufferProperties", (PFN_vkGetPhysicalDeviceExternalBufferProperties)nullptr },
   { "vkGetPhysicalDeviceExternalFencePropertiesKHR", (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)nullptr },
   { "vkGetPhysicalDeviceExternalFenceProperties", (PFN_vkGetPhysicalDeviceExternalFenceProperties)nullptr },
   { "vkGetPhysicalDeviceExternalImageFormatPropertiesNV", (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)nullptr },
   { "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR", (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)nullptr },
   { "vkGetPhysicalDeviceExternalSemaphoreProperties", (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)nullptr },
   { "vkGetPhysicalDeviceFeatures2KHR", (PFN_vkGetPhysicalDeviceFeatures2KHR)nullptr },
   { "vkGetPhysicalDeviceFeatures2", (PFN_vkGetPhysicalDeviceFeatures2)nullptr },
   { "vkGetPhysicalDeviceFeatures", (PFN_vkGetPhysicalDeviceFeatures)nullptr },
   { "vkGetPhysicalDeviceFormatProperties2KHR", (PFN_vkGetPhysicalDeviceFormatProperties2KHR)nullptr },
   { "vkGetPhysicalDeviceFormatProperties2", (PFN_vkGetPhysicalDeviceFormatProperties2)nullptr },
   { "vkGetPhysicalDeviceFormatProperties", (PFN_vkGetPhysicalDeviceFormatProperties)nullptr },
   { "vkGetPhysicalDeviceFragmentShadingRatesKHR", (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)nullptr },
   { "vkGetPhysicalDeviceImageFormatProperties2KHR", (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)nullptr },
   { "vkGetPhysicalDeviceImageFormatProperties2", (PFN_vkGetPhysicalDeviceImageFormatProperties2)nullptr },
   { "vkGetPhysicalDeviceImageFormatProperties", (PFN_vkGetPhysicalDeviceImageFormatProperties)nullptr },
   { "vkGetPhysicalDeviceMemoryProperties2KHR", (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)nullptr },
   { "vkGetPhysicalDeviceMemoryProperties2", (PFN_vkGetPhysicalDeviceMemoryProperties2)nullptr },
   { "vkGetPhysicalDeviceMemoryProperties", (PFN_vkGetPhysicalDeviceMemoryProperties)nullptr },
   { "vkGetPhysicalDeviceMultisamplePropertiesEXT", (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)nullptr },
   { "vkGetPhysicalDeviceOpticalFlowImageFormatsNV", (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)nullptr },
   { "vkGetPhysicalDevicePresentRectanglesKHR", (PFN_vkGetPhysicalDevicePresentRectanglesKHR)nullptr },
   { "vkGetPhysicalDeviceProperties2KHR", (PFN_vkGetPhysicalDeviceProperties2KHR)nullptr },
   { "vkGetPhysicalDeviceProperties2", (PFN_vkGetPhysicalDeviceProperties2)nullptr },
   { "vkGetPhysicalDeviceProperties", (PFN_vkGetPhysicalDeviceProperties)nullptr },
   { "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR", (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)nullptr },
   { "vkGetPhysicalDeviceQueueFamilyProperties2KHR", (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)nullptr },
   { "vkGetPhysicalDeviceQueueFamilyProperties2", (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)nullptr },
   { "vkGetPhysicalDeviceQueueFamilyProperties", (PFN_vkGetPhysicalDeviceQueueFamilyProperties)nullptr },
   { "vkGetPhysicalDeviceSparseImageFormatProperties2KHR", (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)nullptr },
   { "vkGetPhysicalDeviceSparseImageFormatProperties2", (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)nullptr },
   { "vkGetPhysicalDeviceSparseImageFormatProperties", (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)nullptr },
   { "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV", (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)nullptr },
   { "vkGetPhysicalDeviceSurfaceCapabilities2EXT", (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)nullptr },
   { "vkGetPhysicalDeviceSurfaceCapabilities2KHR", (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)nullptr },
   { "vkGetPhysicalDeviceSurfaceCapabilitiesKHR", (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)nullptr },
   { "vkGetPhysicalDeviceSurfaceFormats2KHR", (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)nullptr },
   { "vkGetPhysicalDeviceSurfaceFormatsKHR", (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)nullptr },
   { "vkGetPhysicalDeviceSurfacePresentModesKHR", (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)nullptr },
   { "vkGetPhysicalDeviceSurfaceSupportKHR", (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)nullptr },
   { "vkGetPhysicalDeviceToolPropertiesEXT", (PFN_vkGetPhysicalDeviceToolPropertiesEXT)nullptr },
   { "vkGetPhysicalDeviceToolProperties", (PFN_vkGetPhysicalDeviceToolProperties)nullptr },
   { "vkGetPhysicalDeviceVideoCapabilitiesKHR", (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)nullptr },
   { "vkGetPhysicalDeviceVideoFormatPropertiesKHR", (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)nullptr },
   { "vkGetPipelineCacheData", (PFN_vkGetPipelineCacheData)nullptr },
   { "vkGetPipelineExecutableInternalRepresentationsKHR", (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)nullptr },
   { "vkGetPipelineExecutablePropertiesKHR", (PFN_vkGetPipelineExecutablePropertiesKHR)nullptr },
   { "vkGetPipelineExecutableStatisticsKHR", (PFN_vkGetPipelineExecutableStatisticsKHR)nullptr },
   { "vkGetPipelinePropertiesEXT", (PFN_vkGetPipelinePropertiesEXT)nullptr },
   { "vkGetPrivateDataEXT", (PFN_vkGetPrivateDataEXT)nullptr },
   { "vkGetPrivateData", (PFN_vkGetPrivateData)nullptr },
   { "vkGetQueryPoolResults", (PFN_vkGetQueryPoolResults)nullptr },
   { "vkGetQueueCheckpointData2NV", (PFN_vkGetQueueCheckpointData2NV)nullptr },
   { "vkGetQueueCheckpointDataNV", (PFN_vkGetQueueCheckpointDataNV)nullptr },
   { "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR", (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)nullptr },
   { "vkGetRayTracingShaderGroupHandlesKHR", (PFN_vkGetRayTracingShaderGroupHandlesKHR)nullptr },
   { "vkGetRayTracingShaderGroupHandlesNV", (PFN_vkGetRayTracingShaderGroupHandlesNV)nullptr },
   { "vkGetRayTracingShaderGroupStackSizeKHR", (PFN_vkGetRayTracingShaderGroupStackSizeKHR)nullptr },
   { "vkGetRefreshCycleDurationGOOGLE", (PFN_vkGetRefreshCycleDurationGOOGLE)nullptr },
   { "vkGetRenderAreaGranularity", (PFN_vkGetRenderAreaGranularity)nullptr },
   { "vkGetSamplerOpaqueCaptureDescriptorDataEXT", (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)nullptr },
   { "vkGetSemaphoreCounterValueKHR", (PFN_vkGetSemaphoreCounterValueKHR)nullptr },
   { "vkGetSemaphoreCounterValue", (PFN_vkGetSemaphoreCounterValue)nullptr },
   { "vkGetSemaphoreFdKHR", (PFN_vkGetSemaphoreFdKHR)nullptr },
   { "vkGetShaderBinaryDataEXT", (PFN_vkGetShaderBinaryDataEXT)nullptr },
   { "vkGetShaderInfoAMD", (PFN_vkGetShaderInfoAMD)nullptr },
   { "vkGetShaderModuleCreateInfoIdentifierEXT", (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)nullptr },
   { "vkGetShaderModuleIdentifierEXT", (PFN_vkGetShaderModuleIdentifierEXT)nullptr },
   { "vkGetSwapchainCounterEXT", (PFN_vkGetSwapchainCounterEXT)nullptr },
   { "vkGetSwapchainImagesKHR", (PFN_vkGetSwapchainImagesKHR)nullptr },
   { "vkGetSwapchainStatusKHR", (PFN_vkGetSwapchainStatusKHR)nullptr },
   { "vkGetValidationCacheDataEXT", (PFN_vkGetValidationCacheDataEXT)nullptr },
   { "vkGetVideoSessionMemoryRequirementsKHR", (PFN_vkGetVideoSessionMemoryRequirementsKHR)nullptr },
   { "vkImportFenceFdKHR", (PFN_vkImportFenceFdKHR)nullptr },
   { "vkImportSemaphoreFdKHR", (PFN_vkImportSemaphoreFdKHR)nullptr },
   { "vkInitializePerformanceApiINTEL", (PFN_vkInitializePerformanceApiINTEL)nullptr },
   { "vkInvalidateMappedMemoryRanges", (PFN_vkInvalidateMappedMemoryRanges)nullptr },
   { "vkMapMemory2KHR", (PFN_vkMapMemory2KHR)nullptr },
   { "vkMapMemory", (PFN_vkMapMemory)nullptr },
   { "vkMergePipelineCaches", (PFN_vkMergePipelineCaches)nullptr },
   { "vkMergeValidationCachesEXT", (PFN_vkMergeValidationCachesEXT)nullptr },
   { "vkQueueBeginDebugUtilsLabelEXT", (PFN_vkQueueBeginDebugUtilsLabelEXT)nullptr },
   { "vkQueueBindSparse", (PFN_vkQueueBindSparse)nullptr },
   { "vkQueueEndDebugUtilsLabelEXT", (PFN_vkQueueEndDebugUtilsLabelEXT)nullptr },
   { "vkQueueInsertDebugUtilsLabelEXT", (PFN_vkQueueInsertDebugUtilsLabelEXT)nullptr },
   { "vkQueuePresentKHR", (PFN_vkQueuePresentKHR)nullptr },
   { "vkQueueSetPerformanceConfigurationINTEL", (PFN_vkQueueSetPerformanceConfigurationINTEL)nullptr },
   { "vkQueueSubmit2KHR", (PFN_vkQueueSubmit2KHR)nullptr },
   { "vkQueueSubmit2", (PFN_vkQueueSubmit2)nullptr },
   { "vkQueueSubmit", (PFN_vkQueueSubmit)nullptr },
   { "vkQueueWaitIdle", (PFN_vkQueueWaitIdle)nullptr },
   { "vkRegisterDeviceEventEXT", (PFN_vkRegisterDeviceEventEXT)nullptr },
   { "vkRegisterDisplayEventEXT", (PFN_vkRegisterDisplayEventEXT)nullptr },
   { "vkReleaseDisplayEXT", (PFN_vkReleaseDisplayEXT)nullptr },
   { "vkReleasePerformanceConfigurationINTEL", (PFN_vkReleasePerformanceConfigurationINTEL)nullptr },
   { "vkReleaseProfilingLockKHR", (PFN_vkReleaseProfilingLockKHR)nullptr },
   { "vkReleaseSwapchainImagesEXT", (PFN_vkReleaseSwapchainImagesEXT)nullptr },
   { "vkResetCommandBuffer", (PFN_vkResetCommandBuffer)nullptr },
   { "vkResetCommandPool", (PFN_vkResetCommandPool)nullptr },
   { "vkResetDescriptorPool", (PFN_vkResetDescriptorPool)nullptr },
   { "vkResetEvent", (PFN_vkResetEvent)nullptr },
   { "vkResetFences", (PFN_vkResetFences)nullptr },
   { "vkResetQueryPoolEXT", (PFN_vkResetQueryPoolEXT)nullptr },
   { "vkResetQueryPool", (PFN_vkResetQueryPool)nullptr },
   { "vkSetDebugUtilsObjectNameEXT", (PFN_vkSetDebugUtilsObjectNameEXT)nullptr },
   { "vkSetDebugUtilsObjectTagEXT", (PFN_vkSetDebugUtilsObjectTagEXT)nullptr },
   { "vkSetDeviceMemoryPriorityEXT", (PFN_vkSetDeviceMemoryPriorityEXT)nullptr },
   { "vkSetEvent", (PFN_vkSetEvent)nullptr },
   { "vkSetHdrMetadataEXT", (PFN_vkSetHdrMetadataEXT)nullptr },
   { "vkSetLocalDimmingAMD", (PFN_vkSetLocalDimmingAMD)nullptr },
   { "vkSetPrivateDataEXT", (PFN_vkSetPrivateDataEXT)nullptr },
   { "vkSetPrivateData", (PFN_vkSetPrivateData)nullptr },
   { "vkSignalSemaphoreKHR", (PFN_vkSignalSemaphoreKHR)nullptr },
   { "vkSignalSemaphore", (PFN_vkSignalSemaphore)nullptr },
   { "vkSubmitDebugUtilsMessageEXT", (PFN_vkSubmitDebugUtilsMessageEXT)nullptr },
   { "vkTrimCommandPoolKHR", (PFN_vkTrimCommandPoolKHR)nullptr },
   { "vkTrimCommandPool", (PFN_vkTrimCommandPool)nullptr },
   { "vkUninitializePerformanceApiINTEL", (PFN_vkUninitializePerformanceApiINTEL)nullptr },
   { "vkUnmapMemory2KHR", (PFN_vkUnmapMemory2KHR)nullptr },
   { "vkUnmapMemory", (PFN_vkUnmapMemory)nullptr },
   { "vkUpdateDescriptorSets", (PFN_vkUpdateDescriptorSets)nullptr },
   { "vkUpdateDescriptorSetWithTemplateKHR", (PFN_vkUpdateDescriptorSetWithTemplateKHR)nullptr },
   { "vkUpdateDescriptorSetWithTemplate", (PFN_vkUpdateDescriptorSetWithTemplate)nullptr },
   { "vkUpdateVideoSessionParametersKHR", (PFN_vkUpdateVideoSessionParametersKHR)nullptr },
   { "vkWaitForFences", (PFN_vkWaitForFences)nullptr },
   { "vkWaitForPresentKHR", (PFN_vkWaitForPresentKHR)nullptr },
   { "vkWaitSemaphoresKHR", (PFN_vkWaitSemaphoresKHR)nullptr },
   { "vkWaitSemaphores", (PFN_vkWaitSemaphores)nullptr },
   { "vkWriteAccelerationStructuresPropertiesKHR", (PFN_vkWriteAccelerationStructuresPropertiesKHR)nullptr },
   { "vkWriteMicromapsPropertiesEXT", (PFN_vkWriteMicromapsPropertiesEXT)nullptr }
};

extern const std::vector<dynamic_symbol> vulkan_symbols = {
    { "vkGetInstanceProcAddr", +[](VkInstance instance, const char *name) -> void* {
        if (proc_addr == nullptr) {
            proc_addr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();
        }
        auto it = proc_to_trampoline.find(name);
        if (it == proc_to_trampoline.end()) {
            std::cerr << "vkGetInstanceProcAddr: unknown proc " << name << std::endl;
            assert(!"unknown proc in vkGetInstanceProcAddr");
        } else {
            Trampoline &trampoline = trampoline_get_by_id(it->second);
            if (strcmp(name, "vkCreateInstance") == 0) {
                trampoline.native_func = (void*)+[](VkInstanceCreateInfo *create_info, const VkAllocationCallbacks *pAllocator, VkInstance *instance) -> VkResult {
                    if (global_instance) {
                        *instance = global_instance;
                        return VK_SUCCESS;
                    }

                    assert(pAllocator == nullptr);
                    static PFN_vkCreateInstance create_instance = (PFN_vkCreateInstance)proc_addr(nullptr, "vkCreateInstance");

                    u32 n;
                    if (SDL_Vulkan_GetInstanceExtensions(get_window(), &n, nullptr) != SDL_TRUE) {
                        std::cerr << SDL_GetError() << std::endl;
                        assert(!"SDL_Vulkan_GetInstanceExtensions");
                    }
                    std::vector<const char*> extensions(n);
                    if (SDL_Vulkan_GetInstanceExtensions(get_window(), &n, extensions.data()) != SDL_TRUE) {
                        std::cerr << SDL_GetError() << std::endl;
                        assert(!"SDL_Vulkan_GetInstanceExtensions");
                    }
                    for (u32 i = 0; i < create_info->enabledExtensionCount; i++) {
                        const char *str = create_info->ppEnabledExtensionNames[i];
                        if (strcmp(str, "VK_KHR_android_surface") == 0)
                            continue;
                        bool exists = false;
                        for (auto &it : extensions) {
                            if (strcmp(str, it) == 0) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists)
                            extensions.emplace_back(str);
                    }

                    VkInstanceCreateInfo new_info = *create_info;
                    new_info.enabledExtensionCount = extensions.size();
                    new_info.ppEnabledExtensionNames = extensions.data();

                    VkResult result = create_instance(&new_info, nullptr, instance);
                    device_proc_addr = (PFN_vkGetDeviceProcAddr)proc_addr(*instance, "vkGetDeviceProcAddr");
                    
                    global_instance = *instance;
                    return result;
                };
            } else if (strcmp(name, "vkDestroyInstance") == 0) {
                trampoline.native_func = (void*)+[]([[maybe_unused]] VkInstance instance, [[maybe_unused]] const VkAllocationCallbacks *pAllocator) -> void {
                    // no-op
                };
            } else if (strcmp(name, "vkGetDeviceProcAddr") == 0) {
                trampoline.native_func = (void*)+[](VkDevice device, const char *name) -> void* {
                    auto it = proc_to_trampoline.find(name);
                    if (it == proc_to_trampoline.end()) {
                        std::cerr << "vkGetDeviceProcAddr: unknown proc " << name << std::endl;
                        assert(!"unknown proc in vkGetDeviceProcAddr");
                    } else {
                        Trampoline &trampoline = trampoline_get_by_id(it->second);
                        if (strcmp(name, "vkAcquireNextImageKHR") == 0) {
                            trampoline.native_func = (void*)+[](VkDevice device, VkSwapchainKHR swapchain, u64 timeout, VkSemaphore semaphore, VkFence fence, u32 *pImageIndex) -> VkResult {
                                PFN_vkAcquireNextImageKHR func_addr = (PFN_vkAcquireNextImageKHR)device_proc_addr(device, "vkAcquireNextImageKHR");

                                VkResult result = func_addr(device, swapchain, timeout, semaphore, fence, pImageIndex);
                                // this error code is basically a non-fatal VK_ERROR_OUT_OF_DATE_KHR,
                                // it reports that the surface properties have changed but the swapchain
                                // is still usable. roblox cant recognise this, so we have to convert it
                                if (result == VK_SUBOPTIMAL_KHR)
                                    return VK_SUCCESS;
                                return result;
                            };
                        } else {
                            trampoline.native_func = (void*)device_proc_addr(device, name);
                            if (trampoline.native_func == nullptr) {
                                    return nullptr;
                            }
                        }
                        return trampoline.guest_trampoline;
                    }
                };
            } else if (strcmp(name, "vkCreateAndroidSurfaceKHR") == 0) {
                trampoline.native_func = (void*)+[](VkInstance instance, VkAndroidSurfaceCreateInfoKHR *, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *surface) -> VkResult {
                    assert(pAllocator == nullptr);

                    if (SDL_Vulkan_CreateSurface(get_window(), instance, surface) != SDL_TRUE) {
                        std::cerr << "SDL_Vulkan_CreateSurface " << SDL_GetError() << std::endl;
                        return VK_ERROR_OUT_OF_HOST_MEMORY; // made up
                    }
                    return VK_SUCCESS;
                };
            } else if (strcmp(name, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR") == 0) {
                trampoline.native_func = (void*)+[](VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) -> VkResult {
                    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR func_addr = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)proc_addr(global_instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

                    // we have a slight problem with wayland: the wl surface expects the
                    // the program to determine the current extent by creating the swapchain,
                    // however roblox doesnt understand this
                    // we already have a chosen extent by main() so we'll pass that instead
                    VkResult result = func_addr(physicalDevice, surface, pSurfaceCapabilities);
                    if (result != VK_SUCCESS)
                        return result;
                    VkExtent2D extent = pSurfaceCapabilities->currentExtent;
                    if (extent.width == 0xFFFFFFFF && extent.height == 0xFFFFFFFF) {
                        SDL_GetWindowSize(get_window(), (int*)&extent.width, (int*)&extent.height);
                        pSurfaceCapabilities->currentExtent = extent;
                        pSurfaceCapabilities->minImageExtent = extent;
                        pSurfaceCapabilities->maxImageExtent = extent;
                    }
                    return VK_SUCCESS;
                };
            } else {
                trampoline.native_func = (void*)proc_addr(instance, name);
                if (trampoline.native_func == nullptr) {
                    return nullptr;
                }
            }
            return trampoline.guest_trampoline;
        }
    } }
};

void create_vulkan_trampolines() {
    for (auto &sym : actual_symbols) {
        assert(!sym.arg_types.empty());
        proc_to_trampoline.emplace(sym.name, trampoline_create(sym.name, nullptr, sym.arg_types, sym.is_ellipsis_function, sym.has_execution_side_effects).id + trampoline_id_offset);
    }
}
