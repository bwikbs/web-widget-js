var str = "Hello Wolld";
var matched = str.match(/.ll./g);
print(matched[0]);
print(matched[1]);

var replaced = str.replace(/.ll./g, "abc");
print(replaced);