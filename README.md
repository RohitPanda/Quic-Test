# YouTube test for QUIC library

This work continues [Youtube test][YouTube-test] <br>
The goal of the thesis is testing QUIC performance for YouTube videos for IPv4 and IPv6.<br>

The project consist of 3 projects: lsquic probe client, quic youtube test and modified tcp youtube test.
Tests were build on Ubuntu 16 LTS.

## Quickstart

To install libraries, run bash ./libs_installation.sh for x64_86 or ./libs_installation_cpu_general.sh to build all dependencies from source code.<br>
Script requires installed build-essential tools, especially gcc and make. To download all distributives with wget, please set up all settings like proxy in advance. <br>
QUIC YouTube test requires: lsquic, ffmpeg, libevent.<br>
YouTube test (tcp_youtube_test) requires: ffmgep, curl, yasm, zlib. Curl should be built locally to disable extra features and use the same encryption library for both tests.<br>

For lsquic and curl boringssl is required.<br>
Boringssl is built with go.<br>

Script installs globally if no command is found: zlib, libevent, yasm. Other libraries are installed locally only.<br>
Installing script was tested for only Ubuntu 16 and 18 LTS. For manual installation or script description refer to [Manual installation][Manual installation].

Generated test executables are copied to /tests. Run 'bash get_top_youtube_list.sh' first to generate list of youtube top 50 video ids and then run 'bash test_run.sh' to run all test configurations (ip4 and ipv6 for 35,39,43 quic versions).<br> To run script get_top_youtube_list.sh you need GET command installed.<br>
Test time and used port numbers can be altered in test_run.sh MAX_TIME_ARG and PORTS variables. Results are stored in tests/results.<br>
Tests can be automated by adding with grontab -e content of tests/schedule_tests.txt. It refreshes every day at 12:50 top youtube video list and runs tests every hour. To disable, delete added lines. Change the path according to your test directory location. For instance (cd /home/userName/tests && timeout 1h bash ..) for both lines.

## YouTube test (tcp_youtube_test)

YouTube test is continues [Youtube test][YouTube-test] with use of curl.<br>
New features of test:<br>
<ul>
 <li>Imported on CMake;</li>
 <li>Updated ffmpeg and curl references using boring ssl;</li>
 <li>Extended help support, now it's possible to see output hits with --hekp_output option in addtion to existing help;</li>
 <li>Additionally to tcp connect time, there is full connect time with ssl nearby;</li>
 <li>Support of TLSv1.3 with use of BoringSSl;</li>
 <li>Curl updated to 7.58;</li>
 <li>FFMGPEG updated to 4.0;</li>
 <li>Use of static libraries only (can use binaries)</li>
</ol>

To install test, install all libraries and run cmake & make.
Then run the test with ./youtube_test, use --help

## QUIC YouTube test (youtube_test)

Instead of [Youtube test][YouTube-test] was build test, where curl is exchanged for [QUIC client library][QUIC client].<br>

Repeats features of modified TCP test. There are 2 additional output fields for audio and video stream: quic version and http header version. More information can be requested with ./youtube_test --help_output<br>
To install test, install all libraries, QUIC client library and run cmake & make.<br>
Then run the test with ./youtube_test, use --help

## QUIC client library(quic_probe)

QUIC client library purposed for downloading web resource or receiving download size with head http request. <br>
Based on [lsquic library][LSQUIC].<br>
Life cycle of usage is:<br>
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

## License

[![Creative Commons License][license-image]][license]

This template is licensed under a [Creative Commons Attribution-ShareAlike 4.0 International License][license], meaning that:

 * You can share (copy, redistribute) and adapt (remix, transform, build upon) this template for any purpose, even commercially.
 * If you share the template or a modified (derived) version of it, you must attribute the template to the original authors ([Florian Walch and contributors][template-authors]) by providing a [link to the original template][template-url] and indicate if changes were made.
 * Any derived template has to use the [same][license] or a [compatible license][license-compatible].

The license **applies only to the template**; there are no restrictions on the resulting PDF file or the contents of your thesis.


[YouTube-test]: https://github.com/sabyahsan/Youtube-test
[LSQUIC]: https://github.com/litespeedtech/lsquic-client
[Manual installation]: https://gitlab.lrz.de/cm/2018-sergey-masters-code/wikis/Manual-installation
[QUIC client]: https://gitlab.lrz.de/cm/2018-sergey-masters-code#quic-client-library(quic-probe)
[overleaf]: https://www.overleaf.com/
[tex-se]: https://tex.stackexchange.com/
[license-compatible]: https://creativecommons.org/compatiblelicenses
[license-image]: https://i.creativecommons.org/l/by-sa/4.0/88x31.png
[license]: https://creativecommons.org/licenses/by-sa/4.0/
[template-authors]: https://github.com/fwalch/tum-thesis-latex/graphs/contributors
[template-download]: https://github.com/fwalch/tum-thesis-latex/archive/master.zip
[template-url]: https://github.com/fwalch/tum-thesis-latex
