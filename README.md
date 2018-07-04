# YouTube test for QUIC library

This work continues [Youtube test][YouTube-test]https://github.com/sabyahsan/Youtube-test. <br>
The goal of the thesis is testing QUIC performance for YouTube videos for IPv4 and IPv6.<br>

The project consist of 3 projects: quic_probe, youtube_test and tcp_youtube_test.

## TCP YouTube test

Tcp_youtube_test is old [Youtube test][YouTube-test].<br>
New features of test:<br>
<ul>
 <li>Imported on CMake;</li>
 <li>Updated ffmpeg and curl references using boring ssl;</li>
 <li>Extended help support, now it's possible to see output hits with --hekp_output option in addtion to existing help;</li>
 <li>Additionally to tcp connect time, there is full connect time with ssl nearby;</li>
 <li>Support of TLSv1.3 with use of BoringSSl;</li>
 <li>Curl updated to 7.58;</li>
 <li>FFMGPEG updated to 4.0;</li>
</ol>

To install test, install all libraries and run cmake & make.
Then run the test with ./youtube_test, use --help

## QUIC Probe

QUIC probe is a quic_client library for using it to download web pages or receive download size. <br>
Based on [ls quic library][LSQUIC].<br>
Lifecycle of usage is:<br>
<ol>
 <li>create_client</li>
 <li>for every set of request, can be used multiple times:
    <ol>
     <li>start_downloading</li>
     <li>for every request:
        <ol>
         <li>get_download_result whine not null</li>
         <li>destroy_output_data_container</li>
        </ol>
     </li>
    </ol>
 </li>
 <li>destroy_client</li>
</ol>
Example and test usage can be seen in main.c.<br>

To test library for main.c scenario with gprof run ./profiling_script.sh. Result is stored in profile_test/analysis.txt<br>

To check just donwload size, set is_header option = 1 for request.<br>

To install client, install all libraries and run cmake & make. There will be static library libquic_client.a

## Youtube test (QUIC)

Instead of [Youtube test][YouTube-test] was build test, where curl is changed with quic_client(quic_probe).<br>

Repeats features of modified TCP test. There are 2 additional output fields for audio and video stream: quic version and http header version.<br>
To install test, install all libraries, quic_client and run cmake & make.<br>
Then run the test with ./youtube_test, use --help


## Quickstart

To install libraries, run bash ./libs_installation.sh.<br>
It installs globally: zlib, libevent, yasm. Locally: boringssl, lsquic-client, curl, ffmpeg<br>

## License

[![Creative Commons License][license-image]][license]

This template is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License][license], meaning that:

 * You can share (copy, redistribute) and adapt (remix, transform, build upon) this template for any purpose, even commercially.
 * If you share the template or a modified (derived) version of it, you must attribute the template to the original authors ([Florian Walch and contributors][template-authors]) by providing a [link to the original template][template-url] and indicate if changes were made.
 * Any derived template has to use the [same][license] or a [compatible license][license-compatible].

The license **applies only to the template**; there are no restrictions on the resulting PDF file or the contents of your thesis.


[YouTube-test]: https://github.com/sabyahsan/Youtube-test
[LSQUIC]: https://github.com/litespeedtech/lsquic-client
[overleaf]: https://www.overleaf.com/
[tex-se]: https://tex.stackexchange.com/
[license-compatible]: https://creativecommons.org/compatiblelicenses
[license-image]: https://i.creativecommons.org/l/by-sa/4.0/88x31.png
[license]: https://creativecommons.org/licenses/by-sa/4.0/
[template-authors]: https://github.com/fwalch/tum-thesis-latex/graphs/contributors
[template-download]: https://github.com/fwalch/tum-thesis-latex/archive/master.zip
[template-url]: https://github.com/fwalch/tum-thesis-latex