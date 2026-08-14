// This is an analysis of the Desktop_Gremlin structure, made by Copilot.
// Note that I do not use AI in the actual code itself
// Globals (struct-like)
AnimationStates states;
CurrentFrames frames;
FrameCounts counts;
Settings cfg;
MouseSettings mouse;
double nextRandomActionTime = now + 1s;
bool wasIdleLastFrame = false;
int frameIntervalMs = 1000 / cfg.FrameRate;

// Event handlers (fire immediately on input)
on ProgramStart() {
    MediaManager.PlaySound("intro.wav", cfg.StartingChar);
    states.SetState("Intro"); // initial state in code
}

on MouseEnterSprite() {
    states.SetState("Hover");
    // Play hover sound once when entering, but throttle: MediaManager handles delaySeconds
    MediaManager.PlaySound("hover.wav", cfg.StartingChar, delaySeconds:5);
    frames.Hover = 0;
}

on MouseLeaveSprite() {
    states.SetState("Idle");
    frames.Hover = 0;
}

on RightClickSprite() {
    ResetIdleTimer();
    frames.Click = 0;
    states.UnlockState();
    states.SetState("Click");
    MediaManager.PlaySound("mambo.wav", cfg.StartingChar);
    states.LockState();
}

on LeftClickSprite() { // drag
    ResetIdleTimer();
    states.UnlockState();
    states.SetState("Grab");
    MediaManager.PlaySound("grab.wav", cfg.StartingChar);
    DragMove(); // user drags window
    states.SetState("Idle");
    mouse.FollowCursor = !mouse.FollowCursor; // toggle follow
    frames.Grab = 0;
    if (mouse.FollowCursor) {
        states.SetState("Walking");
        states.LockState();
        MediaManager.PlaySound("run.wav", cfg.StartingChar); // start-walk sound
    }
}

on HotspotClick_EmoteN(n) {
    ResetIdleTimer();
    frames.EmoteN = 0;
    states.UnlockState();
    states.SetState("EmoteN");
    MediaManager.PlaySound("emoteN.wav", cfg.StartingChar);
    states.LockState();
}

on ToggleSleep() {
    if (states.GetState("Sleeping")) {
        states.UnlockState();
        states.SetState("Idle");
        ResetIdleTimer();
    } else {
        states.UnlockState();
        MediaManager.PlaySound("sleep.wav", cfg.StartingChar);
        states.SetState("Sleeping");
        states.LockState();
    }
}

on ToggleCursorFollow() { // keyboard or UI toggle
    StopAllMovement();
    StopRandomMove();
    mouse.FollowCursor = !mouse.FollowCursor;
    if (mouse.FollowCursor) {
        states.UnlockState();
        states.SetState("Walking");
        states.LockState();
        MediaManager.PlaySound("run.wav", cfg.StartingChar); // start-walk sound
    } else {
        states.UnlockState();
        states.SetState("Idle");
    }
}

on SpawnFood() {
    // create target at random location
    MediaManager.PlaySound("food.wav", cfg.StartingChar);
    states.SetState("FollowItem");
    states.LockState();
    FoodFollower.StartFollowing(target);
}

// MovementController.RandomMove() — triggered by random action or manual call
RandomMove() {
    StopRandomMove();
    states.SetState("Random"); // walking-like state for the duration
    compute targetLeft, targetTop within MoveDistance
    stepCount = cfg.WalkDistance;
    dx = (targetLeft - window.Left) / stepCount;
    dy = (targetTop - window.Top) / stepCount;
    every 30ms:
        window.Left += dx;
        window.Top += dy (unless gravity)
        PlayWalkAnimation(dx, dy) // advances WalkLeft/Right/Up/Down frames
    after move finish:
        states.SetState("Idle");
}

// Master frame loop: driven by DispatcherTimer interval = frameIntervalMs
while (true) {
    Sleep(frameIntervalMs);

    // Advance repeatable animations only if their state is active
    frames.Grab  = PlayAnimationIfActive("Grab",    "Actions", frames.Grab, counts.Grab, false);
    frames.Emote1= PlayAnimationIfActive("Emote1",  "Emotes",  frames.Emote1, counts.Emote1, false);
    frames.Emote3= PlayAnimationIfActive("Emote3",  "Emotes",  frames.Emote3, counts.Emote3, false);
    frames.Idle  = PlayAnimationIfActive("Idle",    "Actions", frames.Idle, counts.Idle, false);
    frames.Hover = PlayAnimationIfActive("Hover",   "Actions", frames.Hover, counts.Hover, false);
    frames.Sleep = PlayAnimationIfActive("Sleeping","Actions", frames.Sleep, counts.Sleep, false);
    frames.Pat   = PlayAnimationIfActive("Pat",     "Actions", frames.Pat, counts.Pat, false);

    // Single-run animations (reset on completion)
    frames.Emote4 = PlayAnimationIfActive("Emote4", "Emotes", frames.Emote4, counts.Emote4, true);
    frames.Emote2 = PlayAnimationIfActive("Emote2", "Emotes", frames.Emote2, counts.Emote2, true);
    frames.Intro  = PlayAnimationIfActive("Intro",  "Actions", frames.Intro, counts.Intro, true);
    frames.Outro  = PlayAnimationIfActive("Outro",  "Actions", frames.Outro, counts.Outro, true);
    frames.Click  = PlayAnimationIfActive("Click",  "Actions", frames.Click, counts.Click, true);

    // Advance directional/run animations when walking or moving
    // Handle cursor-follow movement: compute dx,dy to cursor
    if (mouse.FollowCursor && states.GetState("Walking")) {
        compute dx, dy, distance;
        if (distance > cfg.FollowRadius) {
            if (cfg.StraightLine) MoveStraightOnly(dx,dy,distance);
            else MoveDiagonally(dx,dy,distance);
            // Move* functions call PlayDirectionalAnimation(dir) which advances run frames:
            // e.g. frames.Right = SpriteManager.PlayAnimation("runRight","Run", frames.Right, counts.Right)
        } else {
            PlayDirectionalAnimation("Idle"); // set runIdle / walkIdle frames
        }
    }

    // Random actions scheduling (only if Settings.AllowRandomness)
    bool isIdleNow = states.IsCompletelyIdle();
    if (isIdleNow && !wasIdleLastFrame) {
        nextRandomActionTime = now + RandomBetween(cfg.RandomMinInterval, cfg.RandomMaxInterval) seconds;
    }
    if (isIdleNow && now >= nextRandomActionTime) {
        states.SetState("Random");
        int action = rng.Next(0,4);
        switch(action) {
            case 0:
                frames.Click = 0;
                states.UnlockState();
                states.SetState("Click");
                MediaManager.PlaySound("mambo.wav", cfg.StartingChar); // random click sound
                states.LockState();
                break;
            case 1:
            case 2:
            case 3:
                TriggerRandomMove(); // MovementController.RandomMove()
                break;
        }
        nextRandomActionTime = now + RandomBetween(cfg.RandomMinInterval, cfg.RandomMaxInterval);
    }
    wasIdleLastFrame = isIdleNow;
}

// PlayAnimationIfActive: advances a frame only while that state is active
int PlayAnimationIfActive(string stateName, string folder, int currentFrame, int frameCount, bool resetOnEnd) {
    if (!states.GetState(stateName)) return currentFrame;
    int next = SpriteManager_PlayAnimation(stateName, folder, currentFrame, frameCount);
    if (next == -1) { // missing sprite sheet or error
        states.UnlockState();
        states.ResetAllExceptIdle();
        return 0;
    }
    if (resetOnEnd && next == 0) {
        states.UnlockState();
        states.ResetAllExceptIdle();
        if (stateName == "Outro") ExitApplication();
    }
    return next;
}
