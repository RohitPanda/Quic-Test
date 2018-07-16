#!/bin/sh
GET "https://www.googleapis.com/youtube/v3/videos/?regionCode=DE&chart=mostPopular&part=id&maxResults=50&key=AIzaSyCtN6h6M_Mbh4E837cXMJgdPTp8Edgd4e4" | grep -E "id\": \"([A-Za-z0-9_-]*)\"" | grep -oE "[A-Za-z0-9_-]{3,}" > top_list.txt
