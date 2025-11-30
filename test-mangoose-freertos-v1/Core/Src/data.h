/*
 * data.h
 *
 *  Created on: Oct 29, 2025
 *      Author: mar
 */

#ifndef SRC_DATA_H_
#define SRC_DATA_H_

// char *WEBSOCKETS_CONNECT_DATA = "{\"capabilities\":{\"modes\":[\"Mono\",\"Stereo\"],\"speakerConfiguration\":{\"bassCapability\":90.0,\"configuration\":\"\",\"deEmphasisFilter\":[1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0],\"features\":[],\"lfRollOff\":50.0,\"maxLatency\":0.0,\"minLatency\":0.0,\"name\":\"BeoConnect Core\",\"phaseCompensationFilter\":[1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0],\"preEmphasisFilter\":[1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0],\"qualityThreshold\":0.0,\"sensitivity\":90.0,\"type\":\"Main\"},\"version\":1},\"formats\":[{\"encoding\":\"lc3plus\"},{\"encoding\":\"pcm\"}],\"networkType\":\"wired\",\"protocolVersion\":4,\"rtpAddress\":\"%u.%u.%u.%u\",\"rtpPort\":10020,\"serialNumber\":\"22223335\",\"type\":\"pairReq\",\"versionID\":\"5.1.0.241\"}";



char *WEBSOCKETS_CONNECT_DATA = "{\"capabilities\":{\"modes\":[\"Mono\",\"Stereo\"],\"speakerConfiguration\":{\"bassCapability\":90.0,\"configuration\":\"\",\"deEmphasisFilter\":[1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0],\"features\":[],\"lfRollOff\":50.0,\"maxLatency\":0.0,\"minLatency\":0.0,\"name\":\"BeoConnect Core\",\"phaseCompensationFilter\":[1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0],\"preEmphasisFilter\":[1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0],\"qualityThreshold\":0.0,\"sensitivity\":90.0,\"type\":\"Main\"},\"version\":1},\"formats\":[{\"encoding\":\"pcm\"}],\"networkType\":\"wired\",\"protocolVersion\":4,\"rtpAddress\":\"%s\",\"rtpPort\":10020,\"serialNumber\":\"22223335\",\"type\":\"pairReq\",\"versionID\":\"5.1.0.241\"}";

#endif /* SRC_DATA_H_ */
