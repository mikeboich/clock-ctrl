/*  

 Copyright (C) 2016 Michael Boich

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RTC_1.h"

int flw_index = 0;
int n_flw_words = 0;

char *flws[] ={
        "abet","able","ably","abut","aced","aces","ache","achy","acid","acme",
			"acne","acre","acts","adds","adze","aero","afar","agar","aged","ages",
			"agog","ahem","ahoy","aide","aids","ails","aims","airs","airy","ajar",
			"akin","alar","alas","alec","ales","alit","alky","ally","alms","aloe",
			"alps","also","alto","alum","amen","amid","ammo","amok","amps","anew",
			"ante","anti","ants","aped","apes","apex","apse","aqua","arch","arcs",
			"area","aria","arid","arks","arms","army","arts","arty","asea","asks",
			"asps","atom","atop","aunt","aura","auto","aver","avid","avow","away",
			"awed","awes","awls","awns","awol","awry","axed","axel","axes","axis",
			"axle","axon","ayes",
	
			"baas","babe","baby","back","bade","bags","bail","bait","bake","bald",
			"bale","balk","ball","balm","band","bane","bang","bank","bans","barb",
			"bard","bare","barf","bark","barn","bars","base","bash","bask","bass",
			"bath","bats","batt","baud","bawl","bays","bead","beak","beam","bean",
			"bear","beat","beau","beds","beef","been","beep","beer","bees","beet",
			"begs","bell","belt","bend","bent","berg","berm","best","beta","bets",
			"bevy","bias","bibs","bide","bids","bike","bile","bilk","bill","bind",
			"bins","bios","bird","bite","bits","blab","blah","blam","bled","blew",
			"blip","blob","bloc","blot","blow","blue","blur","boar","boas","boat",
			"bobs","bods","body","bogs","bogy","boil","bola","bold","boll","bolo",
			"bolt","bomb","bond","bone","bong","bonk","bony","boob","book","boom",
			"boon","boor","boos","boot","bops","bore","born","boss","both","bout",
			"bowl","bows","boxy","boyo","boys","bozo","brad","brag","bran","bras",
			"brat","bray","bred","brew","brie","brig","brim","brit","bros","brow",
			"brrr","brut","buck","buds","buff","bugs","bulb","bulk","bull","bump",
			"bums","bung","bunk","buns","bunt","buoy","burg","burl","burn","burp",
			"burr","bury","bush","busk","buss","bust","busy","buts","butt","buys",
			"buzz","byes","byte",
	
			"cabs","cads","cafe","cage","cagy","cain","cake","caky","calf","calk",
			"call","calm","came","camp","cams","cane","cant","cape","capo","caps",
			"carb","card","care","carp","cars","cart","casa","case","cash","cask",
			"cast","cats","cave","cavy","caws","cede","cell","cels","celt","cent",
			"cess","chad","chap","char","chat","chaw","chef","chew","chez","chia",
			"chic","chin","chip","chit","chop","chow","chub","chug","chum","ciao",
			"cine","cite","city","clad","clam","clan","clap","claw","clay","clef",
			"clip","clod","clog","clop","clot","club","clue","coal","coat","coax",
			"cobs","coca","cock","coco","coda","code","cods","coed","cogs","coif",
			"coil","coin","coke","cola","cold","colt","coma","comb","come","comp",
			"cone","conk","cons","cook","cool","coon","coop","coos","coot","cope",
			"cops","copy","cord","core","cork","corn","cory","cost","cots","coup",
			"cove","cowl","cows","cozy","crab","crag","cram","crap","craw","crew",
			"crib","crop","crow","crud","crux","cube","cubs","cuds","cued","cues",
			"cuff","cuke","cull","cult","cups","curb","curd","cure","curl","curs",
			"curt","cusk","cusp","cuss","cute","cuts","cyan","cyst","czar",
	
			"dabs","dada","dado","dads","daft","dais","dale","dame","damn","damp",
			"dams","dang","dank","dare","dark","darn","dart","dash","data","date",
			"daub","dawn","days","daze","dead","deaf","deal","dean","dear","debs",
			"debt","deck","deco","deed","deem","deep","deer","dees","deft","defy",
			"deli","dell","demo","dens","dent","deny","desk","dews","dewy","dial",
			"dibs","dice","dick","died","dies","diet","digs","dike","dill","dime",
			"dims","dine","ding","dins","dint","dips","dire","dirt","disc","dish",
			"disk","diss","diva","dive","dock","docs","dodo","doer","does","doff",
			"dogs","dogy","dole","doll","dolt","dome","done","dong","dons","doom",
			"door","dope","dopy","dork","dorm","dory","dose","dote","doth","dots",
			"dour","dove","down","doze","dozy","drab","drag","dram","drat","draw",
			"dray","dreg","drek","drew","drip","drop","drub","drug","drum","drys",
			"dual","dubs","duck","duct","dude","duds","duel","dues","duet","duff",
			"dugs","duke","dull","duly","dumb","dump","dune","dung","dunk","duns",
			"duos","dupe","dusk","dust","duty","dyed","dyer","dyes","dyke","dyne",
	
			"each","earl","earn","ears","ease","east","easy","eats","eave","ebbs",
			"echo","ecru","eddy","edge","edgy","edit","eels","egad","eggs","eggy",
			"egos","ekes","elan","elks","ells","elms","else","emir","emit","emus",
			"ends","envy","eons","epee","epic","eras","ergo","ergs","eros","errs",
			"espy","etch","euro","even","ever","eves","evil","ewes","exam","exec",
			"exes","exit","expo","eyed","eyes",
	
			"face","fact","fade","fads","fail","fair","fake","fall","fame","fang",
			"fans","fare","farm","fart","fast","fate","fats","faux","fava","fave",
			"fawn","faze","feal","fear","feat","feed","feel","fees","feet","fell",
			"felt","fems","fend","fern","feta","fete","fets","feud","fiat","fibs",
			"fido","fief","figs","fila","file","fill","film","find","fine","fink",
			"fire","firm","firs","fish","fist","fits","five","fizz","flab","flag",
			"flak","flap","flat","flaw","flax","flay","flea","fled","flee","flew",
			"flex","flip","flit","floe","flog","flop","flow","flub","flue","flus",
			"flux","foal","foam","fobs","foci","foes","fogs","fogy","foil","fold",
			"folk","fond","font","food","fool","foot","fops","ford","fore","fork",
			"form","fort","foss","foul","four","fowl","foxy","frag","frat","fray",
			"free","fret","frit","frog","from","fuel","fugs","fugu","fuji","full",
			"fume","fumy","fund","funk","furl","furs","fury","fuse","fuss","futz",
			"fuze","fuzz",
	
			"gabs","gads","gaff","gaga","gage","gags","gain","gait","gala","gale",
			"gall","gals","game","gams","gamy","gang","gape","gaps","garb","gash",
			"gasp","gate","gave","gawk","gays","gaze","gear","geek","gees","geez",
			"geld","gels","gems","gene","gent","germ","gets","ghee","gibe","gift",
			"giga","gigs","gild","gill","gilt","gimp","gins","gird","girl","gist",
			"give","glad","glee","glen","glib","glim","glob","glom","glop","glow",
			"glue","glug","glum","glut","gnar","gnat","gnaw","gnus","goad","goal",
			"goat","gobs","gods","goer","goes","gogo","gold","golf","gone","gong",
			"good","goof","goon","goop","goos","gore","gory","gosh","gout","gown",
			"grab","grad","gram","gray","grew","grey","grid","grim","grin","grip",
			"grit","grog","grow","grub","guar","guff","gulf","gull","gulp","gums",
			"gunk","guns","guru","gush","gust","guts","guys","gyms","gyps","gyro",
	
			"hack","hags","haha","hail","hair","hale","half","hall","halo","halt",
			"hams","hand","hang","hank","hard","hare","hark","harm","harp","hash",
			"hasp","hate","hats","haul","have","hawk","haze","hazy","head","heal",
			"heap","hear","heat","heck","heed","heel","heft","heir","held","helm",
			"help","hemp","hems","hens","herb","herd","here","hero","hers","hewn",
			"hews","hick","hide","hied","hies","high","hike","hill","hilt","hind",
			"hint","hips","hire","hiss","hive","hoar","hoax","hobo","hobs","hock",
			"hods","hoed","hoes","hogs","hold","hole","holy","home","homy","hone",
			"honk","hood","hoof","hook","hoop","hoot","hope","hops","horn","hose",
			"host","hots","hour","howl","hows","hubs","hued","hues","huff","huge",
			"hugs","hula","hulk","hull","hump","hums","hung","hunk","huns","hunt",
			"hurl","hurt","hush","husk","huts","hymn","hype","hypo",
	
			"ibis","iced","ices","icky","icon","idea","ides","idle","idly","idol",
			"iffy","ilks","ills","imam","imps","inch","info","inks","inky","inns",
			"into","ions","iota","ires","irid","iris","irks","iron","isle","isms",
			"itch","item",
	
			"jabs","jack","jade","jags","jail","jake","jamb","jams","jane","jars",
			"jato","java","jaws","jays","jazz","jean","jeed","jeep","jeer","jees",
			"jeez","jefe","jell","jerk","jest","jets","jews","jibe","jibs","jigs",
			"jill","jilt","jinx","jism","jive","jobs","jock","joes","joey","jogs",
			"john","join","joke","joky","jolt","josh","jots","jowl","joys","judo",
			"jugs","juju","juke","jump","junk","jury","just","jute","juts",
	
			"kart","kata","kays","kbar","keck","keel","keen","keep","kegs","kelp",
			"keno","kent","kept","kerb","kerf","kern","keys","kick","kids","kill",
			"kiln","kilo","kilt","kind","kine","king","kink","kins","kiss","kite",
			"kits","kiva","kiwi","knap","knee","knew","knit","knob","knot","know",
			"kola","konk","kook","kyak",
	
			"labs","lace","lack","lacy","lade","lads","lady","lags","laid","lain",
			"lair","lake","lama","lamb","lame","lamp","land","lane","laps","lard",
			"lark","lase","lash","lass","last","late","lath","laud","lava","lawn",
			"laws","lays","laze","lazy","lead","leaf","leak","lean","leap","leek",
			"leer","left","legs","leis","lend","lens","lent","lept","less","lest",
			"lets","levy","lewd","leys","liar","libs","lice","lick","lids","lied",
			"lien","lier","lies","lieu","life","lift","like","lilt","lily","limb",
			"lime","limn","limo","limp","line","link","lino","lint","lion","lips",
			"lira","lisp","list","live","load","loaf","loam","loan","lobe","lobo",
			"lobs","loch","loci","lock","loco","lode","loft","loge","logo","logs",
			"loin","loll","lone","long","look","loom","loon","loop","loos","loot",
			"lope","lops","lord","lore","lose","loss","lost","lots","loud","loup",
			"lout","love","lows","luau","lube","luck","lude","luge","lugs","lull",
			"lulu","lump","lums","luna","lune","lung","lunk","lure","lurk","lush",
			"lust","lute","luxe","lynx","lyre",
	
			"mace","mach","mack","macs","made","mads","mage","magi","mags","maid",
			"mail","maim","main","make","male","mall","malt","mama","mana","mane",
			"mans","many","maps","mare","mark","mars","mart","mash","mask","mass",
			"mast","mate","math","mats","matt","maud","maul","maxi","mayo","maze",
			"mead","meal","mean","meat","meek","meet","meld","melt","memo","mend",
			"menu","meow","mere","mesh","mess","meth","mewl","mews","mhos","mica",
			"mice","mick","midi","miff","mike","mild","mile","milk","mill","mils",
			"mime","mind","mine","mini","mink","mint","minx","mire","miss","mist",
			"mite","mitt","moan","moat","mobs","mock","mode","mods","mojo","mold",
			"mole","moll","molt","moms","monk","mono","mood","moon","moor","moos",
			"moot","mope","mops","mopy","more","moss","most","mote","moth","move",
			"mown","mows","much","muck","muff","mugs","mule","mull","mums","mumu",
			"muni","murk","muse","mush","musk","muss","must","mute","mutt","myna",
			"myth",
	
			"nabs","nada","nags","nail","name","nana","nape","naps","narc","nave",
			"navy","nays","neap","near","neat","neck","need","neon","nerd","nest",
			"nets","news","newt","next","nibs","nice","nick","nigh","nill","nine",
			"nips","nite","nits","nobs","node","nods","noel","noes","none","nook",
			"noon","nope","norm","nose","nosh","nosy","note","noun","nova","nows",
			"nubs","nude","nuke","null","numb","nuns","nurd","nuts",
	
			"oafs","oaks","oars","oast","oath","oats","obey","obit","oboe","odds",
			"odes","odor","offs","ogle","ogre","ohms","oils","oily","oink","okay",
			"okra","olds","oldy","oleo","oles","omen","omit","once","ones","only",
			"onto","onus","onyx","oohs","oops","ooze","opal","oped","open","opes",
			"opts","opus","oral","orbs","orca","orcs","ores","orgy","ouch","ours",
			"oust","outs","ouzo","oval","oven","over","ovum","owed","owes","owls",
			"owns","oxen","oxes","oyes","oyez",
	
			"pace","pack","pacs","pact","pads","page","paid","pail","pain","pair",
			"pale","pall","palm","pals","pane","pang","pans","pant","papa","paps",
			"para","pard","pare","park","pars","part","pass","past","pate","path",
			"pats","pave","pawl","pawn","paws","pays","peak","peal","pean","pear",
			"peas","peat","peck","pecs","peds","peed","peek","peel","peen","peep",
			"peer","pees","pegs","pein","pelt","pend","pens","pent","peon","perk",
			"perm","pert","peso","pest","pets","pews","pfft","phat","phew","pica",
			"pick","pics","pied","pier","pies","pigs","pike","pile","pill","pima",
			"pimp","pina","pine","ping","pink","pins","pint","piny","pipe","pips",
			"pish","piss","pita","pith","pits","pity","pixy","plan","plat","play",
			"plea","pled","plod","plop","plot","plow","ploy","plug","plum","plus",
			"pock","pods","poem","poet","poke","poky","pole","poll","polo","pols",
			"poly","pomp","poms","pond","pone","pong","pony","poof","pooh","pool",
			"poop","poor","pope","pops","pore","pork","porn","port","pose","posh",
			"post","posy","pots","pour","pout","pows","pram","prat","pray","prep",
			"prex","prey","prez","prig","prim","prod","prof","prog","prom","prop",
			"pros","prow","psst","pubs","puce","puck","puds","puff","pugs","puke",
			"pull","pulp","puls","puma","pump","punk","puns","punt","puny","pupa",
			"pups","pure","purl","purr","push","puss","puts","putt","pyre",
	
			"quad","quai","quay","quid","quin","quip","quit","quiz","quod",
	
			"race","rack","racy","raft","raga","rage","rags","raid","rail","rain",
			"rake","ramp","rams","rand","rang","rank","rant","raps","rapt","rare",
			"rash","rasp","rate","rats","rave","rays","raze","razz","read","real",
			"ream","reap","rear","rebs","recs","redo","reds","reed","reef","reek",
			"reel","refs","regs","rein","rely","rend","rent","repo","reps","rest",
			"revs","rhea","ribs","rice","rich","rick","ride","rids","rife","riff",
			"rifs","rift","rigs","rile","rims","rind","ring","rink","riot","ripe",
			"rips","rise","risk","ritz","rive","road","roam","roan","roar","robe",
			"robs","rock","rode","rods","roes","roil","role","rolf","roll","romp",
			"roms","roof","rook","room","root","rope","ropy","rose","rosy","rote",
			"rots","roue","rout","rove","rows","rube","rubs","ruby","ruck","rude",
			"rued","ruer","rues","ruff","rugs","ruin","rule","ruly","rump","rums",
			"rune","rung","runs","runt","ruse","rush","rust","ruth","ruts","ryes",
	
			"sack","sacs","sade","safe","saga","sage","sags","said","sail","sake",
			"saki","sale","sall","salt","same","sand","sane","sang","sank","sans",
			"saps","sari","sash","sass","sate","save","sawn","saws","says","scab",
			"scad","scam","scan","scar","scat","scot","scow","scry","scud","scum",
			"scup","scut","seal","seam","sear","seas","seat","secs","sect","seed",
			"seek","seem","seen","seep","seer","sees","self","sell","semi","send",
			"sent","serf","sets","sewn","sews","sext","sexy","shag","shah","sham",
			"shed","shes","shim","shin","ship","shmo","shod","shoe","shoo","shop",
			"shot","show","shun","shut","sibs","sick","sics","side","sift","sigh",
			"sign","sike","silk","sill","silo","silt","simp","sims","sine","sing",
			"sink","sins","sips","sire","sirs","site","sith","sits","size","skag",
			"skas","skee","skew","skid","skim","skin","skip","skis","skit","slab",
			"slag","slam","slap","slat","slaw","slay","sled","slew","slid","slim",
			"slip","slit","slob","sloe","slog","slop","slot","slow","slue","slug",
			"slum","slur","slut","smit","smog","smug","smut","snag","snap","snaw",
			"snip","snit","snob","snog","snot","snow","snub","snug","soak","soap",
			"soar","sobs","sock","soda","sods","sofa","soft","soil","sold","sole",
			"solo","soma","some","sone","song","sons","soon","soot","sops","sore",
			"sort","sots","soul","soup","sour","sous","sown","sows","soya","soys",
			"span","spar","spas","spat","spay","spaz","spec","sped","spew","spin",
			"spit","spot","spry","spud","spun","spur","stab","stag","star","stat",
			"stay","stem","step","stet","stew","stir","stop","stow","stub","stud",
			"stun","stye","subs","such","suck","suds","sued","suer","sues","suet",
			"suit","sulk","sulu","sumo","sump","sums","sung","sunk","suns","supe",
			"sups","sure","surf","suss","swab","swag","swam","swan","swap","swat",
			"sway","swig","swim","swum","sync",
	
			"tabs","tach","tack","taco","tact","tads","tags","tail","take","talc",
			"tale","talk","tall","tame","tamp","tams","tang","tank","tans","taos",
			"tape","taps","tare","taro","tarp","tars","tart","task","tate","tats",
			"taut","taxi","teak","teal","team","tear","teas","teat","teed","teem",
			"teen","tees","tele","tell","temp","tend","tens","tent","term","tern",
			"test","text","than","that","thaw","thee","them","then","they","thin",
			"this","thou","thru","thud","thug","thus","tick","tics","tide","tidy",
			"tied","tier","ties","tiff","tike","tiki","tile","till","tilt","time",
			"tine","ting","tins","tint","tiny","tipi","tips","tire","tiro","toad",
			"toby","toed","toes","tofu","toga","togs","toil","toke","told","toll",
			"tomb","tome","toms","tone","tong","tons","tony","took","tool","toon",
			"toot","tope","tops","tore","torn","toro","torr","tort","tory","tosh",
			"toss","tote","tots","tour","tout","town","tows","toys","tram","trap",
			"tray","tree","trek","trig","trim","trio","trip","trod","trop","trot",
			"troy","true","tsar","tuba","tube","tubs","tuck","tuff","tuft","tugs",
			"tuna","tune","tung","tuns","turf","turk","turn","tush","tusk","tuts",
			"tutu","twas","twig","twin","twit","twos","tyer","tyes","tyke","tyne",
			"type","typo","tyre","tyro","tzar",
	
			"ughs","ugly","ukes","ulna","undo","undy","unit","unto","upon","urbs",
			"urea","urge","uric","urns","ursa","used","user","uses","utas","uvea",
	
			"vail","vain","vale","vamp","vane","vans","vary","vase","vast","vats",
			"veal","veep","veer","vees","veil","vein","vend","vent","verb","very",
			"vest","veto","vets","vial","vibe","vice","vied","vies","view","vile",
			"vine","vino","viny","visa","vise","vita","viva","void","vole","volt",
			"vote","vows",
	
			"wack","wade","wads","waft","wage","wags","waif","wail","wait","wake",
			"wale","walk","wall","wand","wane","want","ward","ware","warm","warn",
			"warp","wars","wart","wary","wash","wasp","watt","wave","wavy","waxy",
			"ways","weak","weal","wean","wear","webs","weds","weed","week","weep",
			"weft","weld","well","welt","wend","went","wept","were","west","wets",
			"wham","whap","what","whee","when","whet","whew","whey","whim","whip",
			"whir","whit","whiz","whoa","whom","whys","wick","wide","wife","wigs",
			"wild","wile","will","wilt","wily","wimp","wind","wine","wing","wink",
			"wino","wins","wipe","wire","wiry","wise","wish","wisp","with","wits",
			"woes","woke","woks","wolf","womb","wonk","wont","wood","woof","wool",
			"woos","word","wore","work","worm","worn","wort","wove","wows","wrap",
			"wren","writ","wuss","wyes",
	
			"yack","yagi","yaks","yams","yang","yank","yaps","yard","yarn","yawn",
			"yaws","yays","yeah","year","yeas","yech","yell","yelp","yens","yeti",
			"yipe","yips","yoga","yogi","yoke","yolk","yond","yoni","yore","your",
			"yowl","yuck","yuks","yule","yups","yurt",
	
			"zags","zany","zaps","zeal","zees","zerk","zero","zest","zigs","zinc",
			"zing","zips","ziti","zits","zone","zonk","zoom","zoos","zori","zzzz"
  };

uint8 sequential_mode=1;

int count_flws(char *the_words[]){
    int n=0;
    while(strcmp(the_words[n],"zzzz")){
        n+=1;
    }
    return n;
}

char *random_word(){
    static int number_of_flws=0;
    if(number_of_flws==0){
        number_of_flws = count_flws(flws);
    }
    return flws[rand() % number_of_flws];
}

char *next_word(){
    //static int index = 0;
    char *result; 
    result = flws[flw_index++];
    if(flw_index >= n_flw_words)
      flw_index = 0;
    return (result);
}

void init_flws(){
    RTC_1_TIME_DATE *t = RTC_1_ReadTime();
    uint seed = 86400*t->DayOfYear + 3600*t->Hour + 60*t->Min + t->Sec;
    srand(seed);
    n_flw_words = count_flws(flws);
    flw_index = seed % n_flw_words;
   
}
