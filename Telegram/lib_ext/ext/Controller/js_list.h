#pragma once

const std::string listScript = R"(
    function ListFromArray(rawArray) {
    var list = new List();
    for (var i = 0; i < rawArray.length; i++) {
        list.push(rawArray[i]);
    }
    return list;
    };
    function List() {
    this.last = null;
    this.length = 0;
    this.addTraits = function(object) {
        object.iterator = function() {
        return new ListIterator(this);
        };
        object.pushContentsOf = function(list) {
        var self = this;
        list.forEach(function(item) {
            self.push(item);
        });
        };
        object.push = function(value) {
        this.last = new ListNode(this.last, value);
        this.length += 1;
        if (value == null) {
            print("trying to push null object into list");
            exit(1);
        }
        //print("added to list, length: " + this.length);
        };
        object.map = function(iterationFunction) {
        var output = new List();
        this.forEach(function(item) {
            output.push(iterationFunction(item));
        });
        return output;
        };
        object.pop = function() {
        if (this.length > 0) {
            var output = this.last.value;
            this.last = this.last.link;
            this.length -= 1;
            return output;
        }
        };
        object.forEach = function(iterationFunction) {
        var iterator = this.iterator();
        var shouldLoop = true;
        while (shouldLoop) {
            var item = iterator.next();
            if (item == null) {
            shouldLoop = false;
            }
            else {
            iterationFunction(item);
            }
        }
        };
        object.indexedForEach = function(iterationFunction) {
        var iterator = this.iterator();
        var shouldLoop = true;
        var index = 0;
        while (shouldLoop) {
            var item = iterator.next();
            if (item == null) {
            shouldLoop = false;
            }
            else {
            iterationFunction(index, item);
            }
            index += 1;
        }
        };
        object.shuffle = function() {
        var temp = [];
        this.forEach(function(item) {
            temp[temp.length] = item;
        });
        for (var i = 0; i < temp.length; i++) {
            var j = Math.randInt(0, temp.length - 1);
            var lhs = temp[i];
            var rhs = temp[j];
            temp[i] = rhs;
            temp[j] = lhs;
        };
        this.clear();
        for (var i = 0; i < temp.length; i++) {
            this.push(temp[i]);
        };
        };
        object.clear = function() {
        while (this.length > 0) {
            this.pop();
        }
        };
        object.remove = function(value) {
        var iterator = this.iterator();
        while (true) {
            var item = iterator.next();
            if (item == null) {
            break;
            }
            else if (item.value == value) {
            iterator.remove();
            }
        }
        };
        object.getFirstMatch = function(matchFunction) {
        var keepSearching = true;
        var iterator = this.iterator();
        while (keepSearching) {
            var item = iterator.next();
            if (item == null) {
            keepSearching = false;
            }
            else if (matchFunction(item)) {
            return item;
            }
        }
        return null;
        };
        object.removeMatchAll = function(matchFunction) {
        var keepSearching = true;
        var iterator = this.iterator();
        while (keepSearching) {
            var item = iterator.next();
            if (item == null) {
            keepSearching = false;
            }
            else if (matchFunction(item)) {
            iterator.remove();
            }
        }
        };
    };
    this.addTraits(this);
    };
    function ListNode(link, value) {
        this.link = link;
        this.value = value;
    };
    function ListIterator(list) {
    this._list = list;
    this._previous = null;
    this._current = null;
    this._next = list.last;
    this.next = function() {
        this._previous = this._current;
        this._current = this._next;
        this._next = this._current.link;
        return this._current.value;
    };
    this.replace = function(value) {
        this._current.value = value;
    };
    this.remove = function() {
        if (this._list.last == this._current) {
            this._list.last = this._current.link;
        }
            this._previous.link = this._current.next;
            this._current = this._previous;
            this._next = this._current.link;
            this._list.length -= 1;
        };
    };
)";