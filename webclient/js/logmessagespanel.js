Ext.ns("WaywardMonitoring");

WaywardMonitoring.LogMessagesPanel = Ext.extend(Ext.grid.GridPanel, {
    initComponent:function() {
        config = {
            store: new Ext.data.JsonStore({
                baseParams: {lightWeight:true,ext: 'js'},
                sortInfo: {field:'timestamp', direction:'ASC'},
                url: 'data/sample-log-messages.js',
                root: 'messages',
                totalProperty: 'messageCount',
                fields: [
                        {name: 'timestamp', type: 'int'},
                        'channel',
                        'message',
                        {name: 'thread_id',  type: 'int'},
                        'thread_name',
                        'source_location',
                    ]
                }),
            columns: [{
                    id: 'id',
                    header: "ID",
                    dataIndex: 'id',
                    width: 25,
                    sortable: true
                },{
                    id: 'timestamp',
                    header: "Timestamp",
                    dataIndex: 'timestamp',
                    width: 75,
                    sortable: true
                },{
                    id: 'channel',
                    header: "Channel",
                    dataIndex: 'channel',
                    width: 75,
                    sortable: true
                },{
                    id: 'message',
                    header: "Message",
                    dataIndex: 'message',
                    width: 420,
                    sortable: true
                },{
                    id: 'thread_name',
                    header: "Thread",
                    dataIndex: 'thread_name',
                    width: 75,
                    sortable: true
                },{
                    id: 'source_location',
                    header: "Source",
                    dataIndex: 'source_location',
                    width: 75,
                    sortable: true
                }]
            }
        Ext.apply(this, Ext.apply(this.initialConfig, config));
        this.bbar = new Ext.PagingToolbar({
                store: this.store,
                pageSize: 500,
                displayInfo: true
            });
        WaywardMonitoring.LogMessagesPanel.superclass.initComponent.apply(this, arguments);
        this.on({
                afterlayout: {scope: this, single: true, fn:function() {
                    this.store.load({params:{start:0, limit:500}});
                }}
            })
    }
});

Ext.reg('wwm.log_messages_panel', WaywardMonitoring.LogMessagesPanel);

