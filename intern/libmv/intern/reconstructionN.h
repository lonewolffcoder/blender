/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2011 Blender Foundation.
 * All rights reserved.
 *
 * Contributor(s): Blender Foundation,
 *                 Tianwei Shen
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef LIBMV_C_API_RECONSTRUCTIONN_H_
#define LIBMV_C_API_RECONSTRUCTIONN_H_

#include "intern/reconstruction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct libmv_ReconstructionN libmv_ReconstructionN;

typedef struct libmv_MultiviewReconstructionOptions {
	int select_keyframes;
	int keyframe1, keyframe2;
	int *all_refine_intrinsics;		/* this should be an array since each clip has its own refine_flags */
} libmv_MultiviewReconstructionOptions;

typedef void (*multiview_reconstruct_progress_update_cb) (void* customdata,
                                                          double progress,
                                                          const char* message);

void libmv_reconstructionNDestroy(libmv_ReconstructionN* libmv_reconstructionN);

libmv_ReconstructionN** libmv_solveMultiviewReconstruction(const int clip_num,
        const struct libmv_TracksN **all_libmv_tracks,
        const libmv_CameraIntrinsicsOptions *all_libmv_camera_intrinsics_options,
        libmv_MultiviewReconstructionOptions* libmv_reconstruction_options,
        multiview_reconstruct_progress_update_cb progress_update_callback,
        void* callback_customdata);

#ifdef __cplusplus
}
#endif

#endif   // LIBMV_C_API_RECONSTRUCTIONN_H_
