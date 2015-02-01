#!/usr/bin/env node


(function(){
    var path = require('path');

    function loadModules(){
        var PLUGINS_PATH = '/usr/lib/webapi-plugins';
        var fs = require('fs');
        var moduleList = [];
        if (fs.existsSync(PLUGINS_PATH)) {
            moduleList = fs.readdirSync(PLUGINS_PATH);
        }

        var globalModules = [];
        var subModules = {};
        for( var i in moduleList ){
            if( path.extname(moduleList[i]) == '.plugin' ){
                var module = moduleList[i].split('.');
                var modulepath = moduleList[i];
                if(module.length == 2){
                    globalModules.push(modulepath);
                }else if( module.length == 3 ){ // only support 2 depth submodule xx.yy.plugin (0) , xx.yy.zz.plugin(x)
                    if( !subModules.hasOwnProperty(module[0]) )
                        subModules[module[0]] = [];
                    subModules[module[0]].push(modulepath);
                }
            }
        }
        function redefineProperty(name,prop){
            if(prop.hasOwnProperty('value')){
                var v = prop.value;
                var isloaded = false;
                delete prop.value;
                prop.get = function(){
                    if( !isloaded ){
                        isloaded = true;
                        for( var i in subModules[name] ){
                            var submodule = subModules[name][i];
                            var submoduleobj = require( path.join(PLUGINS_PATH, submodule) );
                            Object.defineProperties(v, submoduleobj);
                        }
                    }
                    return v;
                }
                if( prop.hasOwnProperty('writable') ){
                    delete prop.writable;
                }
            }
            return prop;
        }

        //load global module
        for( var i in globalModules ){
            var globalmodule = globalModules[i];
            var modulename = globalmodule.split('.')[0];

            var loadedSymbols = require(path.join(PLUGINS_PATH, globalmodule));
            for( var symbol in loadedSymbols ){
                var prop = loadedSymbols[symbol];
                //redefine property description
                prop = redefineProperty(symbol, prop);
                Object.defineProperty(GLOBAL, symbol, prop);
                //Dirty patch for unknown situation.
                //TODO: Remove Dirty Patch
                try {
                    if (symbol == 'tizen')
                        tizen;
                } catch(e) {
                }
            }
        }
    }

    //change cmdline
    process.title = process.argv[1];
    var cmdpath =  process.argv[1].split('/');

    //getting appid from cmdpath
    var appid = cmdpath.pop();
    cmdpath.pop();

    //getting package path
    var packagePath = cmdpath.join('/');

    //Change App privilege
    var serviceUtil = require('/usr/lib/wrt-service/serviceutil.node');
    serviceUtil.setPrivilege(appid.split('.')[0], process.argv[1]);


    // change cwd
    process.chdir(packagePath);
    process.chdir('res/wgt');


    //initialize internal modules

    //dlog enable
    var util = require('util');
    var dlog = require('/usr/lib/wrt-service/nodedlog.node');
    console.log = function(){
        dlog.logd(appid, util.format.apply(this, arguments));
    };
    console.info = function(){
        dlog.logv(appid, util.format.apply(this, arguments));
    };
    console.error = function(){
        dlog.loge(appid, util.format.apply(this, arguments));
    };
    console.warn = console.info;
    console.logd = function(){
        if( arguments.length > 1 ){
            dlog.logd(arguments[0], util.format.apply(this,  Array.prototype.slice.call(arguments,1)) );
        }else{
            dlog.logd(util.format.apply(this, arguments) );
        }
    };
    console.logv = function(){
        if( arguments.length > 1 ){
            dlog.logv(arguments[0], util.format.apply(this,  Array.prototype.slice.call(arguments,1)) );
        }else{
            dlog.logv(util.format.apply(this, arguments) );
        }
    };
    console.loge = function(){
        if( arguments.length > 1 ){
            dlog.loge(arguments[0], util.format.apply(this,  Array.prototype.slice.call(arguments,1)) );
        }else{
            dlog.loge(util.format.apply(this, arguments) );
        }
    };

    //init g main loop
    require('/usr/lib/wrt-service/gcontext.node').init();

    //appfw load
    var appfw = require('/usr/lib/wrt-service/appfw.node');
    //native load
    GLOBAL.native = require('/usr/lib/wrt-service/native.node');
    //ACE module load
    serviceUtil.initAce(appid);
    loadModules();

    //module path update for custom modules
    var modulepaths = [ process.cwd(), process.cwd() + '/node_modules' ];
    module.paths = modulepaths.concat(module.paths);

    //getting start page
    var startScript = serviceUtil.getStartScript(appid);
    if (!startScript) {
        startScript = "index.js";
    }

    //load user script
    var app = require( path.join(packagePath, '/res/wgt/' + startScript));

    appfw.init(process.argv.slice(1));
    appfw.onService = function(bundle) {
        appfw.bundle = bundle;
        if (app.onRequest) {
            app.onRequest();
        }
    };
    appfw.onTerminate = function() { process.exit(); };

    if (app.onExit) {
        process.on('exit',app.onExit);
    }

    if (app.onStart) {
        app.onStart();
    }

    //running main loop

})();
