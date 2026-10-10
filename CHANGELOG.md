## 0.1.0 (2026-10-10)

### Feat

- **note-selection**: some initial work on selection system
- **shader**: optional custom .fs shader that applies to the whole gameplay
- **track**: enwidenlanes rendering stuffs
- **timing-group**: nohead
- **app_config + arc**: colorblind
- **scenecontrol**: enwidencamera!
- **timing_group**: noconnection
- **window**: some progress on general setting window
- **arkohra**: able to reload config without closing app
- **arkohra**: botch up being able to resize with fixed aspect ratio
- **arkohra**: groupalpha kinda works i think (jank af)
- **arkohra**: project setting window initial progress
- **render**: ending arccap
- **arc**: each arc refs previous + next arc for judgement fading purpose
- **scenecontrol**: HIDEGROUP!!!
- **keyframe**: easing calculations + interpolation (both blindly written)
- **arkohra**: inital commit
- **font**: have a fallbank solution to grab the missing glyphs (mostly)
- **render**: fade-in for tap/arctap/arcsegments
- **hud**: add progress bar
- **arcres**: refactor segment shadow rendering to match body and apply tg arcres
- **timing gorup**: parsed name and arcres
- **timing group**: noclip
- **timing group**: add some tg props + minor refactors
- **render**: add beatline rendering
- **arccap**: minor tweaks
- **arc**: added following arccaps
- **arc**: arc head
- **interval tree**: interval tree for hold/arc + tests / other minor fixes
- **height_indicator**: good enough for now
- **height_indicator**: initial setup, not completed
- **test**: initial unit testing stuffs + along-the-way tweaks
- **arkohra**: initial commit

### Fix

- **render**: assigned notes with an id to avoid pointer related issues fetching ntoes
- **apply_chart**: patch bg + jacket loading logic
- **arccap**: divide by 0
- **test**: update test after easing changed to clamping behaviour
- **scenecontrol**: simplify the process and hopefully fixes the default sc channels for good
- **render**: missed an ending arccap gatechecking logic
- **reader**: apparently im not supposed to tweak the default -9999999
- **reader**: fix a thing
- **parsing**: more robust arc parsing
- **hud**: fix progress bar
- **render**: fix sky notes form not clipping correctly
- **chart-reader**: fix a typo that somehow didnt backfire
- **arc**: fix stupid logic + revert arc judgement fade
- **arc**: fix self arc looping + tweak judgement alpha
- **render + arcformula**: change z_pos to double for much more precise renders
- **main**: properly set default texture for jacket img + bg
- **easing**: stupid typo
- **value channel**: fix import
- **hold**: fix incorrect triangle count that might sometimes cause texture glitch
- **main**: i hate string i hate string i hate string i hate string i hate string i hate string
- **render**: fix using wrong list
- **tree**: middle-out tree insert for sorted arrays
- **arkohra**: attempt to fix some pointer-related occasional crashes
- **itv_tree**: incorrect canonicalization placement + new inverted interval data test
- **itv tree**: make sure that the low and high are actually low and high
- **config**:  vscode task (#10)
- **config**:  vscode task

### Refactor

- **window**: move out dra+resize logic so it can be reused
- **window**: refactor panels in app setting
- **main**: refactor out some stuffs
- **window**: some refactoring on rgui inputs for ease of use (for now)
- **arkohra**: not rlly vibing with some abstracted fns
- **app_config**: move some stuffs away from app_config
- **arkohra**: refactor rendering stuffs to help consistency
- **custom-types**: separate data types + tests
