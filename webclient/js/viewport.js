Ext.ns("WaywardMonitoring");

WaywardMonitoring.menu = [
    {
        id: 'welcome-tree-node',
        text: 'Welcome',
        href: '#static/welcome.html',
        leaf: true
    },{
        text: 'Memory',
        expanded: true,
        children: [{
            text: 'Objects',
            xtype: 'wwm.memory_objects_panel',
            leaf: true
        },{
            text: 'Heaps',
            xtype: 'wwm.memory_heaps_panel',
            leaf: true
        }]
    }, {
        text: 'Log Messages',
        xtype: 'wwm.log_messages_panel',
        leaf: true
    }, {
        text: 'Threading / Lock Activity',
        xtype: 'wwm.threading_lock_activity_panel',
        leaf: true
    }]

Ext.onReady(function(){
    Ext.QuickTips.init();

    // NOTE: This is an example showing simple state management. During development,
    // it is generally best to disable state management as dynamically-generated ids
    // can change across page loads, leading to unpredictable results.  The developer
    // should ensure that stable state ids are set for stateful components in real apps.
    Ext.state.Manager.setProvider(new Ext.state.CookieProvider());

    function navigationClickHandler(n) {
        var tabPanel = Ext.getCmp('moduleTabPanel');
        if (true == n.isLeaf()) {
            if (undefined !== n.attributes.xtype) {
                var id = 'module-' + n.attributes.xtype;
                var panel = tabPanel.getComponent(id);
                if (undefined == panel) {
                    panel = tabPanel.add({
                            title: n.attributes.text,
                            id: id,
                            xtype: n.attributes.xtype
                        });
                }
            } else {
                var id = 'module-' + n.attributes.href;
                var panel = tabPanel.getComponent(id);
                if (undefined == panel) {
                    var href = n.attributes.href.substring(1);
                    panel = tabPanel.add(new Ext.Panel({
                            title: n.attributes.text,
                            id: id,
                            autoLoad: href
                        }));
                }
            }
            tabPanel.setActiveTab(panel);
        }
    }
    var headerPanel = new Ext.BoxComponent({
            region: 'north',
            height: 32, // give north and south regions a height
            autoEl: {
                tag: 'div',
                html:'<h1>Wayward Monitoring</h1>'
            }
        })
    var tabPanel = new Ext.TabPanel({
            id: 'moduleTabPanel',
            region: 'center',
            enableTabScroll: true,
            resizeTabs: true,
            minTabWidth: 115,
            tabWidth: 135,
            deferredRender: true,
            activeTab: 0,
            defaults: {
                closable: true
            },
            plugins: new Ext.ux.TabCloseMenu()
        })
    var navigationTreePanel = new Ext.tree.TreePanel({
            region: 'west',
            collapsible: true,
            title: 'Navigation',
            width: 200,
            autoScroll: true,
            split: true,
            rootVisible: false,
            root: new Ext.tree.AsyncTreeNode({
                expanded: true,
                draggable: false,
                children: WaywardMonitoring.menu
            }),
            listeners: {
                click: navigationClickHandler
            }
        })
    var propertyGrid = new Ext.grid.PropertyGrid({
            region: 'east',
            collapsible: true,
            title: 'Properties',
            split: true,
            width: 225, // give east and west regions a width
            minSize: 175,
            maxSize: 400,
            margins: '0 5 0 0',
            source: {
            }
        })
    var statusBar = new Ext.Toolbar({
            region: 'south',
            // autoHeight: true,
            height: 32,
            split: false,
            items:[
                'Status',
                '-',
                '->',
                'Right Status'
            ]
        })
    
    var viewport = new Ext.Viewport({
        layout: 'border',
        items: [
            headerPanel,
            navigationTreePanel,
            tabPanel,
            propertyGrid,
            statusBar
        ]
    });

    setTimeout(function(){
        Ext.get('loading').remove();
        Ext.get('loading-mask').fadeOut({remove:true});
    }, 250);
    var node = navigationTreePanel.getNodeById('welcome-tree-node');
    if (node) {
        node.select();
        navigationClickHandler(node);
    }
});

