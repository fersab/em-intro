// --- mod player ---
const mod_player = new ChiptuneJsPlayer(new ChiptuneJsConfig(-1));
let mod_buffer = null;
let mod_playing = false;
let mod_started = false;

const mod_btn = document.getElementById("mod-playpause");
const mod_title = document.getElementById("mod-title");
const mod_seek = document.getElementById("mod-seek");
const mod_time_el = document.getElementById("mod-time");

function mod_format_time(secs) {
  let m = (secs / 60) | 0;
  let s = (secs % 60) | 0;
  return m + ":" + (s < 10 ? "0" : "") + s;
}

// preload the mod file
mod_player.load("musiklinjen.mod", function(buffer) {
  mod_buffer = buffer;
  mod_title.textContent = "musiklinjen.mod";
});

function mod_start_playback() {
  mod_player.play(mod_buffer);
  mod_started = true;
  mod_playing = true;
  mod_btn.innerHTML = "&#9646;&#9646;";
}

// play/pause toggle
mod_btn.addEventListener("click", function() {
  if (!mod_buffer) return;
  if (!mod_started) {
    // first play — may need to unlock audio context
    if (mod_player.context && mod_player.context.state === "suspended") {
      mod_player.context.resume().then(mod_start_playback);
    } else {
      mod_start_playback();
    }
  } else if (mod_playing) {
    mod_player.togglePause();
    mod_playing = false;
    mod_btn.innerHTML = "&#9654;";
  } else {
    mod_player.togglePause();
    mod_playing = true;
    mod_btn.innerHTML = "&#9646;&#9646;";
  }
});

// seek bar + time display
setInterval(function() {
  if (!mod_buffer || !mod_playing) return;
  let cur = mod_player.getCurrentTime();
  let dur = mod_player.duration();
  if (dur > 0) {
    mod_seek.value = (cur / dur * 1000) | 0;
    mod_time_el.textContent = mod_format_time(cur);
  }
}, 250);
