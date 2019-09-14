/*
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
 */

/** \file
 * \ingroup edtransform
 */

#include "MEM_guardedalloc.h"

#include "DNA_mesh_types.h"
#include "DNA_object_types.h"
#include "DNA_scene_types.h"

#include "BLI_math.h"
#include "BLI_string.h"
#include "BLI_utildefines.h"

#include "BLT_translation.h"

#include "BKE_context.h"
#include "BKE_editmesh.h"
#include "BKE_global.h"
#include "BKE_report.h"
#include "BKE_editmesh.h"
#include "BKE_layer.h"
#include "BKE_mesh.h"
#include "BKE_scene.h"
#include "BKE_unit.h"

#include "RNA_access.h"
#include "RNA_define.h"
#include "RNA_enum_types.h"

#include "WM_api.h"
#include "WM_message.h"
#include "WM_types.h"
#include "WM_toolsystem.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "ED_screen.h"
/* for USE_LOOPSLIDE_HACK only */
#include "ED_mesh.h"

#include "transform.h"
#include "transform_convert.h"
#include "transform_op.h"

const char OP_NORMAL_ROTATION[] = "TRANSFORM_OT_rotate_normal";

/* -------------------------------------------------------------------- */
/* Transform Init (Normal Rotation) */

/** \name Transform Normal Rotation
 * \{ */

static void storeCustomLNorValue(TransDataContainer *tc, BMesh *bm)
{
  BMLoopNorEditDataArray *lnors_ed_arr = BM_loop_normal_editdata_array_init(bm, false);
  // BMLoopNorEditData *lnor_ed = lnors_ed_arr->lnor_editdata;

  tc->custom.mode.data = lnors_ed_arr;
  tc->custom.mode.free_cb = freeCustomNormalArray;
}

void freeCustomNormalArray(TransInfo *t, TransDataContainer *tc, TransCustomData *custom_data)
{
  BMLoopNorEditDataArray *lnors_ed_arr = custom_data->data;

  if (t->state == TRANS_CANCEL) {
    BMLoopNorEditData *lnor_ed = lnors_ed_arr->lnor_editdata;
    BMEditMesh *em = BKE_editmesh_from_object(tc->obedit);
    BMesh *bm = em->bm;

    /* Restore custom loop normal on cancel */
    for (int i = 0; i < lnors_ed_arr->totloop; i++, lnor_ed++) {
      BKE_lnor_space_custom_normal_to_data(
          bm->lnor_spacearr->lspacearr[lnor_ed->loop_index], lnor_ed->niloc, lnor_ed->clnors_data);
    }
  }

  BM_loop_normal_editdata_array_free(lnors_ed_arr);

  tc->custom.mode.data = NULL;
  tc->custom.mode.free_cb = NULL;
}

/* Works by getting custom normal from clnor_data, transform, then store */
static void applyNormalRotation(TransInfo *t, const int UNUSED(mval[2]))
{
  char str[UI_MAX_DRAW_STR];

  float axis_final[3];
  copy_v3_v3(axis_final, t->orient_matrix[t->orient_axis]);

  if ((t->con.mode & CON_APPLY) && t->con.applyRot) {
    t->con.applyRot(t, NULL, NULL, axis_final, NULL);
  }

  FOREACH_TRANS_DATA_CONTAINER (t, tc) {
    BMEditMesh *em = BKE_editmesh_from_object(tc->obedit);
    BMesh *bm = em->bm;

    BMLoopNorEditDataArray *lnors_ed_arr = tc->custom.mode.data;
    BMLoopNorEditData *lnor_ed = lnors_ed_arr->lnor_editdata;

    float axis[3];
    float mat[3][3];
    float angle = t->values[0];
    copy_v3_v3(axis, axis_final);

    snapGridIncrement(t, &angle);

    applySnapping(t, &angle);

    applyNumInput(&t->num, &angle);

    headerRotation(t, str, angle);

    axis_angle_normalized_to_mat3(mat, axis, angle);

    for (int i = 0; i < lnors_ed_arr->totloop; i++, lnor_ed++) {
      mul_v3_m3v3(lnor_ed->nloc, mat, lnor_ed->niloc);

      BKE_lnor_space_custom_normal_to_data(
          bm->lnor_spacearr->lspacearr[lnor_ed->loop_index], lnor_ed->nloc, lnor_ed->clnors_data);
    }

    t->values_final[0] = angle;
  }

  recalcData(t);

  ED_area_status_text(t->sa, str);
}

void initNormalRotation(TransInfo *t)
{
  t->mode = TFM_NORMAL_ROTATION;
  t->transform = applyNormalRotation;

  setInputPostFct(&t->mouse, postInputRotation);
  initMouseInputMode(t, &t->mouse, INPUT_ANGLE);

  t->idx_max = 0;
  t->num.idx_max = 0;
  t->snap[0] = 0.0f;
  t->snap[1] = DEG2RAD(5.0);
  t->snap[2] = DEG2RAD(1.0);

  copy_v3_fl(t->num.val_inc, t->snap[2]);
  t->num.unit_sys = t->scene->unit.system;
  t->num.unit_use_radians = (t->scene->unit.system_rotation == USER_UNIT_ROT_RADIANS);
  t->num.unit_type[0] = B_UNIT_ROTATION;

  FOREACH_TRANS_DATA_CONTAINER (t, tc) {
    BMEditMesh *em = BKE_editmesh_from_object(tc->obedit);
    BMesh *bm = em->bm;

    BKE_editmesh_ensure_autosmooth(em);
    BKE_editmesh_lnorspace_update(em);

    storeCustomLNorValue(tc, bm);
  }
}

/** \} */

void TRANSFORM_OT_rotate_normal(struct wmOperatorType *ot)
{
  /* identifiers */
  ot->name = "Rotate Normals";
  ot->description = "Rotate split normal of selected items";
  ot->idname = OP_NORMAL_ROTATION;
  ot->flag = OPTYPE_REGISTER | OPTYPE_UNDO | OPTYPE_BLOCKING;

  /* api callbacks */
  ot->invoke = transform_invoke;
  ot->exec = transform_exec;
  ot->modal = transform_modal;
  ot->cancel = transform_cancel;
  ot->poll = ED_operator_editmesh;

  RNA_def_float_rotation(
      ot->srna, "value", 0, NULL, -FLT_MAX, FLT_MAX, "Angle", "", -M_PI * 2, M_PI * 2);

  Transform_Properties(ot, P_ORIENT_AXIS | P_ORIENT_MATRIX | P_CONSTRAINT | P_MIRROR);
}