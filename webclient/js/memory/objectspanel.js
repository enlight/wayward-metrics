Ext.ns("WaywardMonitoring");

WaywardMonitoring.MemoryObjectsPanel = Ext.extend(Ext.grid.GridPanel, {
    initComponent:function() {
        config = {
            store: new Ext.data.JsonStore({
                baseParams: {lightWeight:true,ext: 'js'},
                sortInfo: {field:'type', direction:'ASC'},
                url: 'data/sample-memory-objects.js',
                root: 'objects',
                totalProperty: 'objectCount',
                fields: [
                        'type',
                        {name: 'size_total',  type: 'int'},
                        {name: 'size_live',   type: 'int'},
                        {name: 'count_total', type: 'int'},
                        {name: 'count_live',  type: 'int'}
                    ]
                }),
            columns: [{
                    id: 'type',
                    header: "Type",
                    dataIndex: 'type',
                    width: 420,
                    sortable: true
                },{
                    id: 'size_total',
                    header: "Total Size",
                    dataIndex: 'size_total',
                    width: 75,
                    sortable: true
                },{
                    id: 'size_live',
                    header: "Live Size",
                    dataIndex: 'size_live',
                    width: 75,
                    sortable: true
                },{
                    id: 'count_total',
                    header: "Total Count",
                    dataIndex: 'count_total',
                    width: 75,
                    sortable: true
                },{
                    id: 'count_live',
                    header: "Live Count",
                    dataIndex: 'count_live',
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
        WaywardMonitoring.MemoryObjectsPanel.superclass.initComponent.apply(this, arguments);
        this.on({
                afterlayout: {scope: this, single: true, fn:function() {
                    this.store.load({params:{start:0, limit:500}});
                }}
            })
    }
});

Ext.reg('wwm.memory_objects_panel', WaywardMonitoring.MemoryObjectsPanel);

