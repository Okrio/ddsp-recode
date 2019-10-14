#include "m_pd.h"
#include <iostream>

static t_class *ddsp_tilde_class;

#define PD_BLOCK_SIZE 256

typedef struct _ddsp_tilde {
  t_object x_obj;
  t_sample f;

  float *buffer;

  int bufferReadHead;
  int bufferWriteHead;
  int nextCondition;

  t_inlet  *x_in2;
  t_outlet *x_out;
} t_ddsp_tilde;

t_int *ddsp_tilde_perform(t_int *w)
{
  t_ddsp_tilde *x = (t_ddsp_tilde *)(w[1]);
  t_sample  *in1 =    (t_sample *)(w[2]);
  t_sample  *in2 =    (t_sample *)(w[3]);
  t_sample  *out =    (t_sample *)(w[4]);
  int          n =           (int)(w[5]);

  while (n--) *out++ = x->buffer[x->bufferReadHead++];

  // GET NEW SAMPLES AND WRITE THEM TO (POTENTIALLY OVERLAPPING) CURRENT WRITE
  // BUFFER. THREADED ?

  x->bufferReadHead = x->bufferReadHead % (2 * PD_BLOCK_SIZE);
  x->bufferWriteHead = PD_BLOCK_SIZE - x->bufferReadHead;


  return (w+6);
}

void ddsp_tilde_dsp(t_ddsp_tilde *x, t_signal **sp)
{
  dsp_add(ddsp_tilde_perform, 5, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void ddsp_tilde_free(t_ddsp_tilde *x)
{
  free(x->buffer);
  inlet_free(x->x_in2);
  outlet_free(x->x_out);
}

void *ddsp_tilde_new(t_floatarg f)
{
  t_ddsp_tilde *x = (t_ddsp_tilde *)pd_new(ddsp_tilde_class);

  x->buffer = (float *) malloc(2 * PD_BLOCK_SIZE * sizeof(float));

  x->bufferReadHead  = 0;
  x->bufferWriteHead = PD_BLOCK_SIZE;
  x->nextCondition   = 0;

  x->x_in2=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  x->x_out=outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

extern "C" {
void ddsp_tilde_setup(void) {
  ddsp_tilde_class = class_new(gensym("ddsp~"),
        (t_newmethod)ddsp_tilde_new,
        0, sizeof(t_ddsp_tilde),
        CLASS_DEFAULT,
        A_DEFFLOAT, 0);

  class_addmethod(ddsp_tilde_class,
        (t_method)ddsp_tilde_dsp, gensym("dsp"), A_CANT, 0);

  CLASS_MAINSIGNALIN(ddsp_tilde_class, t_ddsp_tilde, f);
}
}
