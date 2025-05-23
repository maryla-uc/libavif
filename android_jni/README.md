# libavif Android JNI Bindings

This subdirectory contains Android JNI bindings that will enable use of the libavif decoder in Android apps.


## Prerequisites

* [Android SDK](https://developer.android.com/studio#downloads) - For building the JNI bindings. SDK has to be downloaded and set up for Android API target 30.
* [Android NDK](https://developer.android.com/ndk/downloads) - For building the decoder (in the examples, we use dav1d).
* [Gradle](https://gradle.org/).
* CMake.
* Ninja.

## Generate the AAR package

The following steps will generate the AAR package that contains libavif and the JNI wrapper. It can then be used as a dependency for Android apps.

> Note: The instructions in this section will use dav1d as the AV1 decoder. Any other decoder can be used but the `avifandroidjni/src/main/jni/CMakeLists.txt` file has to be updated accordingly.

Step 1 - Checkout libavif

```
$ git clone https://github.com/AOMediaCodec/libavif.git
$ cd libavif
```

Step 2 - Set the SDK and NDK paths in environment variables. (Recommended Android NDK revision: r25c)

```
$ export ANDROID_SDK_ROOT="/path/to/android/sdk"
$ export ANDROID_NDK_HOME="/path/to/android/ndk"
```

Step 3 - Checkout and build dav1d (or libgav1)

```
$ cd ext
$ ./dav1d_android.sh "${ANDROID_NDK_HOME}"
$ cd ..
```

If you want to use libgav1 instead:

```
$ cd ext
$ ./libgav1_android.sh "${ANDROID_NDK_HOME}"
$ cd ..
```

Update [CMakeLists.txt](avifandroidjni/src/main/jni/CMakeLists.txt) as follows:
 * Set `AVIF_CODEC_DAV1D` to `OFF`
 * Set `AVIF_CODEC_LIBGAV1` to `LOCAL`.

Step 4 - Checkout and build libyuv

```
$ cd ext
$ ./libyuv_android.sh "${ANDROID_NDK_HOME}"
$ cd ..
```

If you do not want to use libyuv, then update
[CMakeLists.txt](avifandroidjni/src/main/jni/CMakeLists.txt) as follows:
 * Set `AVIF_LIBYUV` to `OFF`.

Step 5 - Build the JNI Wrapper and generate the AAR package

```
$ cd android_jni
$ ./gradlew build assembleRelease
```

If all the steps were completed successfully, the AAR package that contains libavif and the JNI wrapper can be found in `libavif/android_jni/avifandroidjni/build/outputs/aar`. You can now follow the instructions in the next section to include the AAR package as a dependency in your Android project.

## Add an AAR dependency to your Android Project

The instructions on how to add the AAR package as a dependency to your Android project can be found [here](https://developer.android.com/studio/projects/android-library#psd-add-aar-jar-dependency).

## Running Instrumented Tests

Step 1 - Build the library

Make sure to build the library by following the steps under
[Generate the AAR package](#generate-the-aar-package) section above.

Step 2 - Set up a device/emulator

Make sure that a device or an emulator has been set up and is available via
`adb`.

Step 3 - Run the tests

```
$ cd android_jni
$ ./gradlew connectedAndroidTest
```

## Using Android Studio

The entire android_jni directory can be imported as a project into Android Studio.

To build the project from within Android Studio, follow the all the steps from the above section to checkout and build libgav1. After that, the last step is equivalent to invoking the build from Android Studio.

## Maven Releases

Maven hosted version of libavif can be found here:
https://repo1.maven.org/maven2/org/aomedia/avif/android/avif/
