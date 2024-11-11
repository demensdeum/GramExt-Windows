#pragma once

const std::string sdkScript = R"(
    var outputMessageTransformers = new List();
    var OutputMessageAddTransformer = function(transformer) {
        outputMessageTransformers.push(transformer);
    };
    var TransformOutputMessage = function(message) {
        var output = message;
        var transformFunc = function(transformer) {
            output = transformer(output);
        };
        outputMessageTransformers.forEach(transformFunc);
        print(output);
        global_returnValue = output;
    };
)";