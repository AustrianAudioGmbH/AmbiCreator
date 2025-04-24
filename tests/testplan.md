# Test Plan for Austrian Audio AmbiCreator Plugin in REAPER

## Objective

To validate the functionality, performance, and compatibility of the Austrian Audio AmbiCreator plugin within REAPER, ensuring it operates as expected for audio engineers producing Ambisonics B-format signals for immersive audio applications.

## Scope

- **Plugin**: Austrian Audio AmbiCreator (VST3, AU, or AAX format, as available).
- **DAW**: REAPER (version 7.35 or latest stable version as of April 24, 2025).

### Features to Test

- Generation of first-order Ambisonics B-format signals (W, X, Y, Z channels).
- Adjustment of vertical axis (Z-axis) information.
- Horizontal orientation control for virtual head rotation.
- Input and output level metering for all four channels.
- Automation of vertical axis and orientation parameters.
- Plugin stability and performance (CPU usage, latency, crashes).
- Preset management (save, load, and recall).
- Compatibility with REAPER’s routing, rendering, and Ambisonics decoding tools.

## Test Environment

- **OS**: Windows 11 or macOS (latest stable versions).
- **Audio Interface**: Any ASIO-compatible interface for Windows or Core Audio for macOS.
- **Sample Rate**: 48 kHz (standard for testing).
- **Bit Depth**: 24-bit.
- **Buffer Size**: 256 samples (adjust as needed for performance).

## Test Scenarios

### Ambisonics B-format Generation

**Objective**: Verify that AmbiCreator generates a valid first-order Ambisonics B-format signal (W, X, Y, Z) from two OC818 dual-output signals.

**Test Steps**:
1. Load AmbiCreator on a track with four input channels (two mono .wav files simulating front and rear diaphragms of two OC818 microphones).
2. Configure routing to map inputs to AmbiCreator’s four input channels.
3. Check output meters for W (omni), X (horizontal figure-8), Y (vertical figure-8), and Z (vertical axis) signals.
4. Decode the B-format output using a REAPER-compatible Ambisonics decoder (e.g., IEM Plug-in Suite) and listen for spatial accuracy.
5. Render output to verify B-format integrity.

**Expected Result**: All four B-format channels (W, X, Y, Z) are generated correctly, with audible spatial characteristics when decoded, and no artifacts.

### Vertical Axis (Z-axis) Adjustment

**Objective**: Test the ability to adjust the vertical axis information in the B-format signal.

**Test Steps**:
1. Apply AmbiCreator to a four-channel track.
2. Adjust the Z-axis gain parameter from -∞ dB to 0 dB.
3. Monitor changes in the vertical spatial perception using an Ambisonics decoder.
4. Test boundary conditions (e.g., Z-axis muted or maximized).
5. Export rendered audio for analysis.

**Expected Result**: Smooth control over vertical axis contribution, with noticeable changes in spatial height perception and no phase issues.

### Horizontal Orientation Control

**Objective**: Ensure AmbiCreator’s horizontal orientation parameter allows virtual head rotation.

**Test Steps**:
1. Load AmbiCreator and configure a four-channel input.
2. Adjust the horizontal orientation parameter to simulate head rotation (e.g., 0° to 360°).
3. Decode the output and listen for changes in the soundfield’s horizontal placement.
4. Verify that orientation changes are smooth and consistent.

**Expected Result**: Accurate virtual rotation of the soundfield, with no glitches or discontinuities in the spatial image.

### Automation

**Objective**: Validate automation of vertical axis and horizontal orientation parameters in REAPER.

**Test Steps**:
1. Create automation envelopes for Z-axis gain (e.g., -∞ dB to 0 dB over 10 seconds).
2. Automate horizontal orientation (e.g., 0° to 180°).
3. Playback and render the project to verify automation accuracy.

**Expected Result**: Automation executes smoothly, with parameters updating in real-time and rendering correctly.

### Preset Management

**Objective**: Test saving, loading, and recalling AmbiCreator presets.

**Test Steps**:
1. Create a custom preset with specific Z-axis and orientation settings.
2. Save the preset in AmbiCreator.
3. Close and reopen the project, then load the preset.
4. Compare settings and audio output.

**Expected Result**: Presets save and load correctly, retaining all settings.

### Stability and Performance

**Objective**: Assess plugin stability under various conditions.

**Test Steps**:
1. Load AmbiCreator on multiple tracks (e.g., 10 tracks) with four-channel inputs.
2. Monitor CPU usage and latency in REAPER’s Performance Meter.
3. Perform rapid parameter changes and check for crashes or glitches.
4. Render the project and verify output integrity.

**Expected Result**: Stable operation, low CPU usage, and no crashes or audio dropouts.

### Rendering and Export

**Objective**: Ensure AmbiCreator’s B-format output is accurately rendered in exported audio.

**Test Steps**:
1. Render the project with AmbiCreator applied to a four-channel .wav file.
2. Compare rendered B-format audio with real-time playback using an Ambisonics decoder.
3. Export stems with AmbiCreator effects baked in.

**Expected Result**: Rendered audio matches real-time playback, with no loss of spatial quality.

## Test Data

### Dummy .wav Files

- **Mic1_Front.wav**: Mono, 1-minute recording simulating the front diaphragm of the first OC818.
- **Mic1_Rear.wav**: Mono, 1-minute recording simulating the rear diaphragm of the first OC818.
- **Mic2_Front.wav**: Mono, 1-minute recording simulating the front diaphragm of the second OC818.
- **Mic2_Rear.wav**: Mono, 1-minute recording simulating the rear diaphragm of the second OC818.

### Creation

1. Use REAPER’s “Insert > Media File” to generate a 60-second sine wave (440 Hz) or white noise.
2. Export as Mic1_Front.wav (File > Render, select WAV, 48 kHz, 24-bit).
3. Duplicate and apply a slight phase shift (e.g., 180 degrees) using REAPER’s FX (e.g., JS:PhaseRotator) for Mic1_Rear.wav.
4. Create Mic2_Front.wav with a different frequency (e.g., 880 Hz) or amplitude (-3 dB) to simulate the second microphone.
5. Apply a phase shift for Mic2_Rear.wav.
6. Place all .wav files in the same directory as the .RPP file.

### Notes

- The FX section assumes AmbiCreator is installed as a VST3 plugin. Adjust the plugin name or format (e.g., AU, AAX) based on your setup.
- The `NCHAN 4` setting ensures tracks support four-channel audio for B-format output.
- The automation envelope (`PARMENV`) is a placeholder; actual parameter names (e.g., “ZAxisGain”) depend on AmbiCreator’s exposed parameters in REAPER.
- File paths for .wav files (Mic1_Front.wav, etc.) should be relative to the project directory for portability.

## Checklist for Setup

- **Install REAPER**:
  - Download and install the latest version of REAPER from [www.reaper.fm](https://www.reaper.fm).
  - Ensure REAPER is associated with .RPP files during installation.
- **Install AmbiCreator**:
  - Download AmbiCreator (VST3, AU, or AAX) from [austrian.audio](https://austrian.audio).
  - Install the plugin and verify it appears in REAPER’s FX browser (VST3 folder).
  - Note: macOS M1/M2 users may need to run REAPER in Rosetta mode unless a native AmbiCreator version is available.
- **Create Dummy .wav Files**:
  - Open REAPER and create a new project.
  - Go to Insert > Media File and generate a 60-second sine wave (440 Hz) or white noise.
  - Export as Mic1_Front.wav (File > Render, select WAV, 48 kHz, 24-bit).
  - Duplicate the track, apply a phase shift (e.g., JS:PhaseRotator) for Mic1_Rear.wav.
  - Create Mic2_Front.wav with a different frequency (e.g., 880 Hz) or amplitude (-3 dB).
  - Apply a phase shift for Mic2_Rear.wav.
  - Configure the Reaper Project to place all .wav files in the same directory as the .RPP file.
- **Create the Reaper Project (.RPP) File**:
  - Create a new .RPP (Reaper Project).
  - Save as AmbiCreator_Test.rpp in the same directory as the .wav files.
  - Open in REAPER to verify tracks and files load correctly.
- **Configure Tracks**:
  - Open AmbiCreator_Test.rpp in REAPER.
  - Ensure the “B-Format Test” track has four mono items (Mic1_Front.wav, Mic1_Rear.wav, Mic2_Front.wav, Mic2_Rear.wav) routed to a four-channel track with AmbiCreator.
  - Verify the “Z-Axis and Orientation Test” track has the same four mono items with AmbiCreator applied.
  - Check the “Automation Test” track for an automation envelope on the Z-axis gain parameter.
  - Install an Ambisonics decoder (e.g., IEM Plug-in Suite) to monitor B-format output.
- **Run Test Scenarios**:
  - Follow the test steps outlined in the “Test Scenarios” section.
  - Use REAPER’s monitoring (solo tracks, adjust levels) and an Ambisonics decoder to evaluate spatial audio output.
  - Record or render outputs for comparison (File > Render).
- **Monitor Performance**:
  - Open REAPER’s Performance Meter (View > Performance Meter) to monitor CPU usage.
  - Check for latency or glitches during playback and rendering.
- **Document Results**:
  - Note any discrepancies, crashes, or unexpected behaviors.
  - Save rendered B-format audio files and screenshots of AmbiCreator settings for reference.

## Assumptions and Limitations

- The test assumes access to AmbiCreator and simulated dual-output .wav files for two OC818 microphones. If no OC818s are available, dummy files simulate the dual-output mode.
- Automation parameter names (e.g., “ZAxisGain”) are placeholders; actual names must be confirmed in REAPER’s automation lane.
- The Reaper Project file is designed for four-channel tracks to support B-format output and may need adjustments for specific AmbiCreator parameters or additional test cases.
- macOS users running M1/M2 systems should use Rosetta for AmbiCreator unless a native version is available.
- An Ambisonics decoder (e.g., IEM Plug-in Suite) is required to audition the B-format output.

## Deliverables

- **AmbiCreator_Test.rpp**: REAPER project file for testing.
- **Mic1_Front.wav, Mic1_Rear.wav, Mic2_Front.wav, Mic2_Rear.wav**: Dummy audio files.
- **Test results document**: Summary of test outcomes, including B-format audio renders and screenshots.
- **Setup checklist**: As provided above.

## Additional Notes

- **Backup and Version Control**: Save the .RPP file frequently to avoid data loss. REAPER automatically creates .RPP-bak files for recovery.
- **Plugin Updates**: Ensure AmbiCreator is the latest version, as Austrian Audio may release updates with new features.
- **Community Resources**: Check the REAPER subreddit or Cockos forums for additional tips on Ambisonics plugin integration.
- **Ambisonics Decoding**: Use a third-party decoder like the IEM Plug-in Suite for accurate monitoring, as REAPER’s native tools may not fully support B-format decoding.

This test plan provides a structured approach to validate the AmbiCreator plugin in REAPER, with a practical .RPP file and setup instructions tailored for Ambisonics production. 
