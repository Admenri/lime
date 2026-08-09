#include "binding_audio.h"

#include "src/audio.h"

namespace binding {

MRB_FUNC(Audio_SetupMIDI) {
  auto* self_obj = lime::Audio::Instance();
  EXC_BEGIN {
    self_obj->SetupMIDI();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_BGMPlay) {
  auto* self_obj = lime::Audio::Instance();

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 1) {
      // bgm_play(filename)
      const char* filename;
      mrb_get_args(mrb, "z", &filename);
      self_obj->BGMPlay(filename);
    } else if (argc == 2) {
      // bgm_play(filename, volume)
      const char* filename;
      mrb_int volume;
      mrb_get_args(mrb, "zi", &filename, &volume);
      self_obj->BGMPlay(filename, volume);
    } else if (argc == 3) {
      // bgm_play(filename, volume, pitch)
      const char* filename;
      mrb_int volume, pitch;
      mrb_get_args(mrb, "zii", &filename, &volume, &pitch);
      self_obj->BGMPlay(filename, volume, pitch);
    } else if (argc == 4) {
      // bgm_play(filename, volume, pitch, pos)
      const char* filename;
      mrb_int volume, pitch;
      mrb_float pos;
      mrb_get_args(mrb, "ziif", &filename, &volume, &pitch, &pos);
      self_obj->BGMPlay(filename, volume, pitch, static_cast<float>(pos));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_BGMStop) {
  auto* self_obj = lime::Audio::Instance();
  EXC_BEGIN {
    self_obj->BGMStop();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_BGMFade) {
  auto* self_obj = lime::Audio::Instance();
  mrb_int time;
  mrb_get_args(mrb, "i", &time);

  EXC_BEGIN {
    self_obj->BGMFade(time);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_BGMPos) {
  auto* self_obj = lime::Audio::Instance();
  EXC_BEGIN {
    return mrb_float_value(mrb, static_cast<mrb_float>(self_obj->BGMPos()));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_BGSPlay) {
  auto* self_obj = lime::Audio::Instance();

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 1) {
      // bgs_play(filename)
      const char* filename;
      mrb_get_args(mrb, "z", &filename);
      self_obj->BGSPlay(filename);
    } else if (argc == 2) {
      // bgs_play(filename, volume)
      const char* filename;
      mrb_int volume;
      mrb_get_args(mrb, "zi", &filename, &volume);
      self_obj->BGSPlay(filename, volume);
    } else if (argc == 3) {
      // bgs_play(filename, volume, pitch)
      const char* filename;
      mrb_int volume, pitch;
      mrb_get_args(mrb, "zii", &filename, &volume, &pitch);
      self_obj->BGSPlay(filename, volume, pitch);
    } else if (argc == 4) {
      // bgs_play(filename, volume, pitch, pos)
      const char* filename;
      mrb_int volume, pitch;
      mrb_float pos;
      mrb_get_args(mrb, "ziif", &filename, &volume, &pitch, &pos);
      self_obj->BGSPlay(filename, volume, pitch, static_cast<float>(pos));
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_BGSStop) {
  auto* self_obj = lime::Audio::Instance();
  EXC_BEGIN {
    self_obj->BGSStop();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_BGSFade) {
  auto* self_obj = lime::Audio::Instance();
  mrb_int time;
  mrb_get_args(mrb, "i", &time);

  EXC_BEGIN {
    self_obj->BGSFade(time);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_BGSPos) {
  auto* self_obj = lime::Audio::Instance();
  EXC_BEGIN {
    return mrb_float_value(mrb, static_cast<mrb_float>(self_obj->BGSPos()));
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_MEPlay) {
  auto* self_obj = lime::Audio::Instance();

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 1) {
      // me_play(filename)
      const char* filename;
      mrb_get_args(mrb, "z", &filename);
      self_obj->MEPlay(filename);
    } else if (argc == 2) {
      // me_play(filename, volume)
      const char* filename;
      mrb_int volume;
      mrb_get_args(mrb, "zi", &filename, &volume);
      self_obj->MEPlay(filename, volume);
    } else if (argc == 3) {
      // me_play(filename, volume, pitch)
      const char* filename;
      mrb_int volume, pitch;
      mrb_get_args(mrb, "zii", &filename, &volume, &pitch);
      self_obj->MEPlay(filename, volume, pitch);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_MEStop) {
  auto* self_obj = lime::Audio::Instance();
  EXC_BEGIN {
    self_obj->MEStop();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_MEFade) {
  auto* self_obj = lime::Audio::Instance();
  mrb_int time;
  mrb_get_args(mrb, "i", &time);

  EXC_BEGIN {
    self_obj->MEFade(time);
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_SEPlay) {
  auto* self_obj = lime::Audio::Instance();

  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 1) {
      // se_play(filename)
      const char* filename;
      mrb_get_args(mrb, "z", &filename);
      self_obj->SEPlay(filename);
    } else if (argc == 2) {
      // se_play(filename, volume)
      const char* filename;
      mrb_int volume;
      mrb_get_args(mrb, "zi", &filename, &volume);
      self_obj->SEPlay(filename, volume);
    } else if (argc == 3) {
      // se_play(filename, volume, pitch)
      const char* filename;
      mrb_int volume, pitch;
      mrb_get_args(mrb, "zii", &filename, &volume, &pitch);
      self_obj->SEPlay(filename, volume, pitch);
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Audio_SEStop) {
  auto* self_obj = lime::Audio::Instance();
  EXC_BEGIN {
    self_obj->SEStop();
  }
  EXC_END(mrb);
  return mrb_nil_value();
}

void InitAudioBinding(mrb_state* mrb) {
  auto mod = mrb_define_module(mrb, "Audio");

  mrb_define_module_function(mrb, mod, "setup_midi", Audio_SetupMIDI,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "bgm_play", Audio_BGMPlay,
                             MRB_ARGS_ANY());
  mrb_define_module_function(mrb, mod, "bgm_stop", Audio_BGMStop,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "bgm_fade", Audio_BGMFade,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "bgm_pos", Audio_BGMPos,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "bgs_play", Audio_BGSPlay,
                             MRB_ARGS_ANY());
  mrb_define_module_function(mrb, mod, "bgs_stop", Audio_BGSStop,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "bgs_fade", Audio_BGSFade,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "bgs_pos", Audio_BGSPos,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "me_play", Audio_MEPlay, MRB_ARGS_ANY());
  mrb_define_module_function(mrb, mod, "me_stop", Audio_MEStop,
                             MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mod, "me_fade", Audio_MEFade,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mod, "se_play", Audio_SEPlay, MRB_ARGS_ANY());
  mrb_define_module_function(mrb, mod, "se_stop", Audio_SEStop,
                             MRB_ARGS_NONE());
}

}  // namespace binding
